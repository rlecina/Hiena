#pragma  once

namespace hiena
{
	template <int Length>
	class CompileTimeString
	{
		struct Storage
		{
			char Str[Length];

			consteval Storage(const char(&Text)[Length])
			{
				for(int i=0; i<Length; i++)
				{
					Str[i] = Text[i];
				}
			}

			consteval Storage(const char* Text, int TextLength)
			{
				int CopiedLength = TextLength < (Length - 1)? TextLength : (Length - 1);
				int Index = 0;
				while(Index < CopiedLength)
				{
					Str[Index] = Text[Index];
					++Index;
				}
				while(Index < Length)
				{
					Str[Index++] = 0;
				}
			}

			consteval Storage(const char* Text1, int Size1, const char* Text2, int Size2)
			{
				int Index = 0;
				for(int i = 0; i < Size1 - 1; i++)
				{
					Str[Index++] = Text1[i];
				}
				for(int i = 0; i < Size2; i++)
				{
					Str[Index++] = Text2[i];
				}
			}

		};
	public:
		consteval CompileTimeString(const char(&Text)[Length])
		:Content(Text)
		{
		}

		consteval CompileTimeString(const char* Text, int TextLength)
		:Content(Text, TextLength)
		{
		}

		consteval CompileTimeString(const char* Text1, int Size1, const char* Text2, int Size2)
		:Content(Text1, Size1, Text2, Size2)
		{
		}

		consteval const char* c_str() const { return &Content.Str[0];}
		consteval int length() const { return Length - 1; }
		consteval int empty() const { return Length == 0 || Content.Str[0] == '\0'; }
		consteval char operator[](int Index) const { return Content.Str[Index]; }

		consteval const char* cbegin() const { return &Content.Str[0];}
		consteval const char* cend() const { return cbegin() + Length;}
		consteval const char* begin() const { return &Content.Str[0];}
		consteval const char* end() const { return cbegin() + Length;}

		const Storage Content;
	};

	template <int Length>
	CompileTimeString(const char(&Text)[Length]) -> CompileTimeString<Length>;

	template <int Length1, int Length2>
	inline consteval auto operator+(const CompileTimeString<Length1>& Lhs, const CompileTimeString<Length2>& Rhs)
	{
		return CompileTimeString<Length1 + Length2 - 1>(Lhs.c_str(), Length1, Rhs.c_str(), Length2);
	}

	template <int Length1, int Length2>
	inline consteval auto operator+(const char(&Lhs)[Length1], const CompileTimeString<Length2>& Rhs)
	{
		return CompileTimeString<Length1 + Length2 - 1>(Lhs, Length1, Rhs.c_str(), Length2);
	}

	template <int Length1, int Length2>
	inline consteval auto operator+(const CompileTimeString<Length1>& Lhs, const char (&Rhs)[Length2])
	{
		return CompileTimeString<Length1 + Length2 - 1>(Lhs.c_str(), Length1, Rhs, Length2);
	}

	namespace CompileTime
	{
		template<CompileTimeString String>
		inline consteval auto operator ""_cs()
		{
			return String;
		}

		template <auto Source, auto ToFind>
		consteval auto CountTimes()
		{
			if constexpr (ToFind.empty())
			{
				return 0;
			}
			else
			{
				int TimesFound = 0;
				int CompareToIndex = 0;
				for(auto c: Source)
				{
					if (c != ToFind[CompareToIndex++])
					{
						CompareToIndex = 0;
					}
					else if (CompareToIndex == ToFind.length())
					{
						++TimesFound;
						CompareToIndex = 0;
					}
				}
				return TimesFound;
			}
		}

		template <auto Source, auto ToFind, auto ReplaceBy>
		consteval auto ReplaceAll()
		{
			if constexpr (ToFind.empty())
			{
				return Source;
			}
			else
			{
				constexpr int NewLength = Source.length() + 1 + CountTimes<Source, ToFind>() * (ReplaceBy.length() - ToFind.length());
				char NewContent[NewLength];
				char* Dst = NewContent;
				auto CompareToIndex = 0;

				for(auto c: Source)
				{
					*Dst++ = c;
					if (c != ToFind[CompareToIndex++])
					{
						CompareToIndex = 0;
					}
					else if (CompareToIndex == ToFind.length())
					{
						Dst -= ToFind.length();
						for (int Idx = 0; Idx < ReplaceBy.length(); ++Idx)
						{
							*Dst++ = ReplaceBy.c_str()[Idx];
						}
						CompareToIndex = 0;
					}
				}
				return CompileTimeString<NewLength>(NewContent);
			}
		}

		template <int LengthSource, int LengthToFind>
		consteval int FindLast(CompileTimeString<LengthSource> Source, CompileTimeString<LengthToFind> ToFind, int Index = -1)
		{
			if (ToFind.empty())
			{
				return -1;
			}
			else
			{
				if (Index < 0 || Index >= Source.length())
				{
					Index = Source.length() - 1;
				}

				int CompareToIndex = ToFind.length() - 1;

				while (Index >= 0)
				{
					if (Source[Index] != ToFind[CompareToIndex])
					{
						CompareToIndex = ToFind.length();
					}
					else if(CompareToIndex == 0)
					{
						return Index;
					}
					--Index;
					--CompareToIndex;
				}
				return -1;
			}
		}

		template <int Begin, int End, int LengthSource>
		consteval auto Substring(CompileTimeString<LengthSource> Source)
		{
			static_assert(Begin <= End, "Wrong indexing");
			static_assert(Begin < LengthSource, "Wrong indexing");
			static_assert(End < LengthSource, "Wrong indexing");
			return CompileTimeString<End - Begin + 1>(&Source.Content.Str[Begin], End - Begin + 1);
		}

		template <auto Source, auto PrefixTag, auto PostfixTag>
		consteval auto ChopLastTagged()
		{
			constexpr auto StartN = FindLast(Source, PrefixTag) + PrefixTag.length();
			constexpr auto EndN = FindLast(Source, PostfixTag);
			return Substring<StartN, EndN>(Source);
		}
	}
}