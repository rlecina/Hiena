#pragma  once

template <int Length>
class CompileTimeString
{
public:
	constexpr CompileTimeString(const char(&Text)[Length])
	{
		for(int i=0; i<Length; i++)
		{
			Content[i] = Text[i];
		}
	}

	constexpr CompileTimeString(const char* Text, int TextLength)
	{
		int CopiedLength = TextLength < (Length - 1)? TextLength : (Length - 1);
		int Index = 0;
		while(Index < CopiedLength)
		{
			Content[Index] = Text[Index];
			++Index;
		}
		while(Index < Length)
		{
			Content[Index++] = 0;
		}
	}

	constexpr CompileTimeString(const char* text1, int size1, const char*text2, int size2)
	{
		int Index = 0;
		for(int i = 0; i < size1 - 1; i++)
		{
			Content[Index++] = text1[i];
		}
		for(int i = 0; i < size2; i++)
		{
			Content[Index++] = text2[i];
		}
	}

	constexpr const char* cbegin() const { return &Content[0];}
	constexpr const char* cend() const { return cbegin() + Length;}
	constexpr const char* begin() const { return &Content[0];}
	constexpr const char* end() const { return cbegin() + Length;}
	constexpr char* begin() { return &Content[0];}
	constexpr char* end() { return begin() + Length;}

	char Content[Length];
};

template<CompileTimeString String>
inline constexpr auto operator ""_cs()
{
	return String;
}

template <int Length1, int Length2>
inline consteval auto operator+(const CompileTimeString<Length1>& Lhs, const CompileTimeString<Length2>& Rhs)
{
	return CompileTimeString<Length1 + Length2 - 1>(Lhs.Content, Length1, Rhs.Content, Length2);
}

template <int Length1, int Length2>
inline consteval auto operator+(const char(&Lhs)[Length1], const CompileTimeString<Length2>& Rhs)
{
	return CompileTimeString<Length1 + Length2 - 1>(Lhs, Length1, Rhs.Content, Length2);
}

template <int Length1, int Length2>
inline consteval auto operator+(const CompileTimeString<Length1>& Lhs, const char (&Rhs)[Length2])
{
	return CompileTimeString<Length1 + Length2 - 1>(Lhs.Content, Length1, Rhs, Length2);
}