#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace Utils
{
	const char *VA(const char *fmt, ...);
	std::string StrToLower(std::string input);
	bool EndsWith(std::string haystack, std::string needle);
	std::vector<std::string> Explode(const std::string& str, char delim);
	void Replace(std::string &string, std::string find, std::string replace);
	bool StartsWith(std::string haystack, std::string needle);
	void EraseSubstring(std::string &base, std::string replace);
	unsigned int OneAtATime(const char *key, size_t len);
	std::string &LTrim(std::string &s);
	std::string &RTrim(std::string &s);
	std::string &Trim(std::string &s);

	std::string FormatTimeSpan(int milliseconds);
	std::string ParseChallenge(std::string data);

	void CreateDir(std::string dir);
	bool FileExists(std::string file);
	void WriteFile(std::string file, std::string data);
	std::string ReadFile(std::string file);

	template<class T>
	rapidjson::Value MakeJsonArray(T* arr, size_t length, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator);

	bool MemIsSet(void* mem, char chr, size_t length);

	template <typename T> bool CompareNumber(T num1, T num2)
	{
		return std::abs(num1 - num2) < std::numeric_limits<T>::epsilon();
	}

	class InfoString
	{
	public:
		InfoString() {};
		InfoString(std::string buffer) : InfoString() { this->Parse(buffer); };
		InfoString(const InfoString &obj) : KeyValuePairs(obj.KeyValuePairs) {};

		void Set(std::string key, std::string value);
		std::string Get(std::string key);

		std::string Build();

		void Dump();

	private:
		std::map<std::string, std::string> KeyValuePairs;
		void Parse(std::string buffer);
	};

	template <typename T> void Merge(std::vector<T>* target, T* source, size_t length)
	{
		if (source)
		{
			for (size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T> void Merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto &entry : source)
		{
			target->push_back(entry);
		}
	}

	template <typename T> using Slot = std::function<T>;
	template <typename T>
	class Signal
	{
	public:
		void connect(Slot<T> slot)
		{
			slots.push_back(slot);
		}

		void clear()
		{
			slots.clear();
		}

		template <class ...Args>
		void operator()(Args&&... args) const
		{
			for (auto& slot : slots)
			{
				slot(std::forward<Args>(args)...);
			}
		}

	private:
		std::vector<Slot<T>> slots;
	};

	template<class T>
	rapidjson::Value MakeJsonArray(T* arr, size_t length, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator)
	{
		rapidjson::Value jsonArr(rapidjson::kArrayType);

		for (size_t i = 0; i < length; i++)
		{
			rapidjson::Value value(arr[i]);
			jsonArr.PushBack(value, allocator);
		}

		return jsonArr;
	}
}
