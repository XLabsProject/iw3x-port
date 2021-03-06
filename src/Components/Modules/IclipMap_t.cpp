#include "STDInclude.hpp"

#define IW4X_CLIPMAP_VERSION 2

namespace Components
{
	Game::IW4::SModelAabbNode* IclipMap_t::BuildSModelNodes(Utils::Memory::Allocator* allocator, Game::IW3::clipMap_t* clipMap, unsigned short* size)
	{
		if (clipMap->numStaticModels == 0) return nullptr;

		float maxs[3];
		float mins[3];

		maxs[0] = clipMap->staticModelList[0].absmax[0];
		maxs[1] = clipMap->staticModelList[1].absmax[1];
		maxs[2] = clipMap->staticModelList[2].absmax[2];

		mins[0] = clipMap->staticModelList[0].absmin[0];
		mins[1] = clipMap->staticModelList[1].absmin[1];
		mins[2] = clipMap->staticModelList[2].absmin[2];

		for (unsigned int i = 1; i < clipMap->numStaticModels; i++)
		{
			maxs[0] = max(maxs[0], clipMap->staticModelList[i].absmax[0]);
			maxs[1] = max(maxs[1], clipMap->staticModelList[i].absmax[1]);
			maxs[2] = max(maxs[2], clipMap->staticModelList[i].absmax[2]);

			mins[0] = min(mins[0], clipMap->staticModelList[i].absmin[0]);
			mins[1] = min(mins[1], clipMap->staticModelList[i].absmin[1]);
			mins[2] = min(mins[2], clipMap->staticModelList[i].absmin[2]);
		}

		Game::IW4::SModelAabbNode* node = allocator->allocate<Game::IW4::SModelAabbNode>();
		node->bounds.compute(mins, maxs);
		node->childCount = static_cast<short>(clipMap->numStaticModels);
		node->firstChild = 0;

		*size = 1;
		return node;
	}

	void IclipMap_t::Dump(Game::IW3::clipMap_t* clipMap)
	{
		if (!clipMap) return;

		Utils::Memory::Allocator allocator;

		Utils::Stream buffer;
		buffer.saveArray("IW4xClip", 8);
		buffer.saveObject(IW4X_CLIPMAP_VERSION);
		buffer.saveString(clipMap->name);

		buffer.saveObject(clipMap->planeCount);
		buffer.saveObject(clipMap->numStaticModels);
		buffer.saveObject(clipMap->numMaterials);
		buffer.saveObject(clipMap->numBrushSides);
		buffer.saveObject(clipMap->numBrushEdges);
		buffer.saveObject(clipMap->numNodes);
		buffer.saveObject(clipMap->numLeafs);
		buffer.saveObject(clipMap->leafbrushNodesCount);
		buffer.saveObject(clipMap->numLeafBrushes);
		buffer.saveObject(clipMap->numLeafSurfaces);
		buffer.saveObject(clipMap->vertCount);
		buffer.saveObject(clipMap->triCount);
		buffer.saveObject(clipMap->borderCount);
		buffer.saveObject(clipMap->partitionCount);
		buffer.saveObject(clipMap->aabbTreeCount);
		buffer.saveObject(clipMap->numSubModels);
		buffer.saveObject(clipMap->numBrushes);
		buffer.saveObject(clipMap->dynEntCount[0]);
		buffer.saveObject(clipMap->dynEntCount[1]);

		std::unordered_map<void*, int> indexMap;

		auto addToMap = [&indexMap](void* ptr, int index)
		{
			if (indexMap.count(ptr))
			{
				Logger::Print("Error: trying to remap pointer to different index!\n");
				return;
			}

			indexMap[ptr] = index;
		};

		auto getFromMap = [&indexMap](void* ptr)
		{
			if (indexMap.count(ptr))
			{
				return indexMap[ptr];
			}

			Logger::Print("Error: missing expected pointer in map\n");

			if (IsDebuggerPresent())
			{
				__debugbreak();
			}

			return -1;
		};


		if (clipMap->planes)
		{
			buffer.saveObject<void*>(clipMap->planes);

			for (int i = 0; i < clipMap->planeCount; ++i)
			{
				addToMap(&clipMap->planes[i], i);
				buffer.saveObject(clipMap->planes[i]);
			}
		}

		if (clipMap->staticModelList)
		{
			for (unsigned int i = 0; i < clipMap->numStaticModels; ++i)
			{
				if (clipMap->staticModelList[i].xmodel)
				{
					buffer.saveString(clipMap->staticModelList[i].xmodel->name);
				}
				else
				{
					buffer.saveString("NONE");
				}

				buffer.saveArray(clipMap->staticModelList[i].origin, 3);
				buffer.saveArray(clipMap->staticModelList[i].invScaledAxis, 3); // fuck how this gets counted by c++
				Game::IW4::Bounds convertedBounds;
				convertedBounds.compute(clipMap->staticModelList[i].absmin, clipMap->staticModelList[i].absmax);
				buffer.saveObject(convertedBounds);
			}
		}

		if (clipMap->materials)
		{
			buffer.saveArray(clipMap->materials, clipMap->numMaterials);
		}

		if (clipMap->brushsides)
		{
			for (unsigned int i = 0; i < clipMap->numBrushSides; ++i)
			{
				addToMap(&clipMap->brushsides[i], i);
				buffer.saveObject(getFromMap(clipMap->brushsides[i].plane));
				buffer.saveObject(clipMap->brushsides[i].materialNum);
				buffer.saveObject(clipMap->brushsides[i].firstAdjacentSideOffset);
				buffer.saveObject(clipMap->brushsides[i].edgeCount);
			}
		}

		if (clipMap->brushEdges)
		{
			for (unsigned int i = 0; i < clipMap->numBrushEdges; ++i)
			{
				addToMap(&clipMap->brushEdges[i], i);
			}

			buffer.saveArray(clipMap->brushEdges, clipMap->numBrushEdges);
		}

		if (clipMap->nodes)
		{
			for (unsigned int i = 0; i < clipMap->numNodes; ++i)
			{
				buffer.saveObject(getFromMap(clipMap->nodes[i].plane));
				buffer.saveObject(clipMap->nodes[i].children[0]);
				buffer.saveObject(clipMap->nodes[i].children[1]);
			}
		}

		if (clipMap->leafs)
		{
			for (unsigned int i = 0; i < clipMap->numLeafs; ++i)
			{
				Game::IW4::cLeaf_t leaf;
				AssertSize(Game::IW4::cLeaf_t, 40);
				std::memcpy(&leaf, &clipMap->leafs[i], sizeof Game::IW4::cLeaf_t);

				leaf.bounds.compute(clipMap->leafs[i].mins, clipMap->leafs[i].maxs);

				buffer.saveObject(leaf);
			}
		}

		if (clipMap->leafbrushNodes)
		{
			for (unsigned int i = 0; i < clipMap->leafbrushNodesCount; ++i)
			{
				buffer.saveObject(clipMap->leafbrushNodes[i]);
				if (clipMap->leafbrushNodes[i].leafBrushCount > 0)
				{
					buffer.saveArray(clipMap->leafbrushNodes[i].data.leaf.brushes, clipMap->leafbrushNodes[i].leafBrushCount);
				}
			}
		}

		if (clipMap->leafbrushes)
		{
			buffer.saveArray(clipMap->leafbrushes, clipMap->numLeafBrushes);
		}

		if (clipMap->leafsurfaces)
		{
			buffer.saveArray(clipMap->leafsurfaces, clipMap->numLeafSurfaces);
		}

		if (clipMap->verts)
		{
			buffer.save(clipMap->verts, 12, clipMap->vertCount);
		}

		if (clipMap->triIndices)
		{
			buffer.save(clipMap->triIndices, 6, clipMap->triCount);
		}

		if (clipMap->triEdgeIsWalkable)
		{
			// TODO: verify this count is correct for iw3
			buffer.saveArray(clipMap->triEdgeIsWalkable, 4 * ((3 * clipMap->triCount + 31) >> 5));
		}

		if (clipMap->borders)
		{
			for (int i = 0; i < clipMap->borderCount; ++i)
			{
				addToMap(&clipMap->borders[i], i);
				buffer.saveObject(clipMap->borders[i]);
			}
		}

		if (clipMap->partitions)
		{
			int borderTotal = 0;
			for (int i = 0; i < clipMap->partitionCount; ++i)
			{
				borderTotal += clipMap->partitions[i].borderCount;
				buffer.saveObject(clipMap->partitions[i].triCount);
				buffer.saveObject(clipMap->partitions[i].borderCount);
				buffer.saveObject(clipMap->partitions[i].firstTri);
				if (clipMap->partitions[i].borderCount > 0)
				{
					buffer.saveObject(getFromMap(clipMap->partitions[i].borders));
				}
			}
			if (borderTotal != clipMap->borderCount)
			{
				Logger::Print("ClipMap: didn't use all borders\n");
			}
		}

		if (clipMap->aabbTrees)
		{
			for (int i = 0; i < clipMap->aabbTreeCount; ++i)
			{
				Game::IW4::CollisionAabbTree aabbTree;

				std::memcpy(aabbTree.midPoint, clipMap->aabbTrees[i].origin, sizeof(aabbTree.midPoint));
				std::memcpy(aabbTree.halfSize, clipMap->aabbTrees[i].halfSize, sizeof(aabbTree.halfSize));
				aabbTree.materialIndex = clipMap->aabbTrees[i].materialIndex;
				aabbTree.childCount = clipMap->aabbTrees[i].childCount;
				aabbTree.u = clipMap->aabbTrees[i].u;

				buffer.saveObject(aabbTree);
			}
		}

		if (clipMap->cmodels)
		{
			for (unsigned int i = 0; i < clipMap->numSubModels; ++i)
			{
				Game::IW4::cmodel_t model;
				AssertSize(Game::IW4::cmodel_t, 68);
				std::memcpy(&model, &clipMap->cmodels[i], sizeof Game::IW4::cmodel_t);

				model.bounds.compute(clipMap->cmodels[i].mins, clipMap->cmodels[i].maxs);
				model.leaf.bounds.compute(clipMap->cmodels[i].leaf.mins, clipMap->cmodels[i].leaf.maxs);

				buffer.saveObject(model);
			}
		}

		if (clipMap->brushes)
		{
			unsigned int brushSideTotal = 0;
			for (unsigned short i = 0; i < clipMap->numBrushes; ++i)
			{
				buffer.saveObject(clipMap->brushes[i].numsides);
				brushSideTotal += clipMap->brushes[i].numsides;
				if (clipMap->brushes[i].numsides > 0)
				{
					buffer.saveObject(getFromMap(clipMap->brushes[i].sides));
				}

				buffer.saveObject(getFromMap(clipMap->brushes[i].baseAdjacentSide));

				for (int x = 0; x < 2; ++x)
				{
					for (int y = 0; y < 3; ++y)
					{
						buffer.saveObject(clipMap->brushes[i].axialMaterialNum[x][y]);
					}
				}

				for (int x = 0; x < 2; ++x)
				{
					for (int y = 0; y < 3; ++y)
					{
						buffer.saveObject(clipMap->brushes[i].firstAdjacentSideOffsets[x][y]);
					}
				}

				for (int x = 0; x < 2; ++x)
				{
					for (int y = 0; y < 3; ++y)
					{
						buffer.saveObject(clipMap->brushes[i].edgeCount[x][y]);
					}
				}
			}

			if (brushSideTotal != clipMap->numBrushSides)
			{
				Logger::Error("didn't use all brush sides\n");
			}

			// brushbounds
			for (unsigned short i = 0; i < clipMap->numBrushes; ++i)
			{
				Game::IW4::Bounds bounds;
				bounds.compute(clipMap->brushes[i].mins, clipMap->brushes[i].maxs);
				buffer.saveObject(bounds);
			}

			// brushcontents
			for (int i = 0; i < clipMap->numBrushes; ++i)
			{
				buffer.saveObject(clipMap->brushes[i].contents);
			}
		}

		// skip mapents here

		for (int n = 0; n < 2; ++n)
		{
			if (clipMap->dynEntDefList[n])
			{
				for (int i = 0; i < clipMap->dynEntCount[n]; ++i)
				{
					buffer.saveObject(clipMap->dynEntDefList[n][i].type);
					buffer.saveObject(clipMap->dynEntDefList[n][i].pose);
					if (clipMap->dynEntDefList[n][i].xModel)
					{
						buffer.saveString(clipMap->dynEntDefList[n][i].xModel->name);
						AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_XMODEL, { clipMap->dynEntDefList[n][i].xModel });
					}
					else
					{
						buffer.saveString("NONE");
					}

					buffer.saveObject(clipMap->dynEntDefList[n][i].brushModel);
					buffer.saveObject(clipMap->dynEntDefList[n][i].physicsBrushModel);

					if (clipMap->dynEntDefList[n][i].destroyFx)
					{
						buffer.saveString(clipMap->dynEntDefList[n][i].destroyFx->name);
					}
					else
					{
						buffer.saveString("NONE");
					}

					// This never works!
					if (clipMap->dynEntDefList[n][i].destroyPieces)
					{
						Components::Logger::Print("Saving pieces %s\n", clipMap->dynEntDefList[n][i].destroyPieces->name);
						int pieces = clipMap->dynEntDefList[n][i].destroyPieces->numpieces;
						for (int piecesIndex = 0; piecesIndex < pieces; piecesIndex++)
						{
							//buffer.saveString(clipMap->dynEntDefList[n][i].xModel->name);
							AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_XMODEL, { clipMap->dynEntDefList[n][i].destroyPieces->pieces[piecesIndex].model });
							Components::Logger::Print("Saving XModel piece %s\n", clipMap->dynEntDefList[n][i].destroyPieces->pieces[piecesIndex].model->name);
						}
					}
					else
					{
						//buffer.saveString("NONE");
					}
					if (clipMap->dynEntDefList[n][i].physPreset)
					{
						buffer.saveString(clipMap->dynEntDefList[n][i].physPreset->name);
					}
					else
					{
						buffer.saveString("NONE");
					}
					buffer.saveObject(clipMap->dynEntDefList[n][i].health);
					buffer.saveObject(clipMap->dynEntDefList[n][i].mass);
					buffer.saveObject(clipMap->dynEntDefList[n][i].contents);
				}
			}
		}

		unsigned short numNodes = 0;
		Game::IW4::SModelAabbNode* nodes = IclipMap_t::BuildSModelNodes(&allocator, clipMap, &numNodes);

		buffer.saveObject(numNodes);
		buffer.saveArray<Game::IW4::SModelAabbNode>(nodes, numNodes);

		////////////////////
		// Version 2
#pragma warning( push )
#pragma warning( disable : 4127 )
		if (IW4X_CLIPMAP_VERSION >= 2)
		{
#pragma warning( pop )
			// add triggers to mapEnts
			if (clipMap->cmodels)
			{

				std::vector<Game::IW4::TriggerSlab>* slabs = new std::vector<Game::IW4::TriggerSlab>();

				for (unsigned short i = 0; i < clipMap->numSubModels; ++i)
				{
					Game::IW4::TriggerHull trigHull{};
					Game::IW4::Bounds cmodelBounds{};
					cmodelBounds.compute(clipMap->cmodels[i].mins, clipMap->cmodels[i].maxs);

					trigHull.bounds = cmodelBounds;
					trigHull.contents = clipMap->cmodels[i].leaf.brushContents | clipMap->cmodels[i].leaf.terrainContents;;

					Game::IW4::TriggerModel trigMod{};
					trigMod.hullCount = 1;
					trigMod.firstHull = i;
					trigMod.contents = clipMap->cmodels[i].leaf.brushContents | clipMap->cmodels[i].leaf.terrainContents;;

					auto* node = &clipMap->leafbrushNodes[clipMap->cmodels[i].leaf.leafBrushNode];

					if (node->leafBrushCount)
					{
						for (int j = 0; j < node->leafBrushCount; ++j)
						{
							auto* brush = &clipMap->brushes[node->data.leaf.brushes[j]];

							auto baseSlab = slabs->size();
							for (unsigned int k = 0; k < brush->numsides; ++k)
							{
								Game::IW4::TriggerSlab curSlab;
								curSlab.dir[0] = brush->sides[k].plane->normal[0];
								curSlab.dir[1] = brush->sides[k].plane->normal[1];
								curSlab.dir[2] = brush->sides[k].plane->normal[2];
								curSlab.halfSize = brush->sides[k].plane->dist;
								curSlab.midPoint = 0.0f; // ??

								slabs->push_back(curSlab);
							}

							trigHull.firstSlab = (unsigned short)baseSlab;
							trigHull.slabCount = (unsigned short)(slabs->size() - baseSlab);
						}
					}

					buffer.saveObject(trigMod);
					buffer.saveObject(trigHull);
				}

				// Save slabs
				buffer.save(slabs->size());
				for (unsigned int i = 0; i < slabs->size(); i++)
				{
					Game::IW4::TriggerSlab slab = (*slabs)[i];
					buffer.saveObject(slab);
				}

				delete slabs;
			}
		}


		///////////////////////

		buffer.saveObject(clipMap->checksum);

		std::string basename(clipMap->name);

		basename.erase(0, 8);
		basename.erase(basename.end() - 7, basename.end());

		Utils::WriteFile(Utils::VA("%s/clipmap/%s.iw4xClipMap", AssetHandler::GetExportPath().data(), basename.c_str()), buffer.toBuffer());

		AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_MAP_ENTS, { clipMap->mapEnts });
	}

	IclipMap_t::IclipMap_t()
	{
		Command::Add("dumpclipMap_t", [](Command::Params params)
			{
				if (params.Length() < 2) return;
				IclipMap_t::Dump(Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_CLIPMAP_PVS, params[1]).clipMap);
			});
	}

	IclipMap_t::~IclipMap_t()
	{

	}
}
