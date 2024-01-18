#pragma once

#include <tuple>
#include <type_traits>

#include "Hiena/meta/Helpers.hpp"
#include "Hiena/meta/Preprocessor.hpp"
#include "Hiena/utility/CompileTimeString.hpp"

namespace hiena
{
	namespace detail
	{
		template <typename T, typename Is, typename = void>
		struct HasFieldCountImpl
		{
			inline static constexpr bool Value = false;
		};

		template <typename T, size_t... Is>
		struct HasFieldCountImpl< T,
								  std::index_sequence<Is...>,
							  	  std::void_t<decltype(T{(void(Is), WildcardType{})...})>>
		{
			inline static constexpr bool Value = true;
		};

		template <typename T, size_t Count>
		inline constexpr bool HasAtLeastFields = HasFieldCountImpl<T, std::make_index_sequence<Count>>::Value;

		template <auto T>
		struct NameOfFieldImpl
		{
			// Expected __PRETTY_FUNCTION__ for field myField in class instance SampleObject of class MyClass in comments
#ifdef __clang__
			// static auto hiena::detail::NameOfFieldImpl<Wrapper<int *>{&SampleObject.myField}>::Get() [T = Wrapper<int *>{&SampleObject.myField}]
			//                                                                                                                           ^       ^^
			static constexpr auto StartTag = CompileTimeString(".");
			static constexpr auto EndTag = CompileTimeString("}]");
#elif defined(__GNUC__) || defined(__GNUG__)
			// static constexpr auto hiena::detail::NameOfFieldImpl<T>::Get() [with auto T = Wrapper<int*>{(& hiena::detail::SampleObject<MyClass>.MyClass::myField)}]
			//                                                                                                                                            ^^       ^^^
			static constexpr auto StartTag = CompileTimeString("::");
			static constexpr auto EndTag = CompileTimeString(")}]");
#endif

			static constexpr auto Get()
			{
				return CompileTime::GetLastBetweenTags<CompileTimeString(__PRETTY_FUNCTION__), StartTag, EndTag>();
			}
		};

		template <class T>
		T SampleObject;
	}

	template <class T, int Count>
	constexpr bool HasFieldCount = detail::HasAtLeastFields<T, Count> && !detail::HasAtLeastFields<T, Count + 1>;

	template <class T>
	constexpr auto ToTuple(T& Obj)
	{
		if constexpr (HasFieldCount<T, 0>)
		{
			return std::tie();
		}
#define HIENA_SUPPORT_FIELD_COUNT(COUNT) \
		else if constexpr (HasFieldCount<T, COUNT>) \
		{ \
			auto& [ HIENA_PP_IDS(e, COUNT) ] = Obj; \
			return std::tie(HIENA_PP_IDS(e, COUNT)); \
		}
		HIENA_SUPPORT_FIELD_COUNT(1)
		HIENA_SUPPORT_FIELD_COUNT(2)
		HIENA_SUPPORT_FIELD_COUNT(3)
		HIENA_SUPPORT_FIELD_COUNT(4)
		HIENA_SUPPORT_FIELD_COUNT(5)
		HIENA_SUPPORT_FIELD_COUNT(6)
		HIENA_SUPPORT_FIELD_COUNT(7)
		HIENA_SUPPORT_FIELD_COUNT(8)
		HIENA_SUPPORT_FIELD_COUNT(9)
		HIENA_SUPPORT_FIELD_COUNT(10)
		HIENA_SUPPORT_FIELD_COUNT(11)
		HIENA_SUPPORT_FIELD_COUNT(12)
		HIENA_SUPPORT_FIELD_COUNT(13)
		HIENA_SUPPORT_FIELD_COUNT(14)
		HIENA_SUPPORT_FIELD_COUNT(15)
		HIENA_SUPPORT_FIELD_COUNT(16)
		HIENA_SUPPORT_FIELD_COUNT(17)
		HIENA_SUPPORT_FIELD_COUNT(18)
		HIENA_SUPPORT_FIELD_COUNT(19)
		HIENA_SUPPORT_FIELD_COUNT(20)
		HIENA_SUPPORT_FIELD_COUNT(21)
		HIENA_SUPPORT_FIELD_COUNT(22)
		HIENA_SUPPORT_FIELD_COUNT(23)
		HIENA_SUPPORT_FIELD_COUNT(24)
		HIENA_SUPPORT_FIELD_COUNT(25)
		HIENA_SUPPORT_FIELD_COUNT(26)
		HIENA_SUPPORT_FIELD_COUNT(27)
		HIENA_SUPPORT_FIELD_COUNT(28)
		HIENA_SUPPORT_FIELD_COUNT(29)
		HIENA_SUPPORT_FIELD_COUNT(30)
		HIENA_SUPPORT_FIELD_COUNT(31)
		HIENA_SUPPORT_FIELD_COUNT(32)
		else
		{
			static_assert(AlwaysFalse<T>, "Too much fields");
		}
#undef HIENA_SUPPORT_FIELD_COUNT
	}

	template <typename... T, typename C>
	constexpr void TransformTuple(const std::tuple<T...>& Fields, C&& Callable)
	{
		return [&]<auto... Idx>(std::index_sequence<Idx...>)
			{
				return Callable(std::get<Idx>(Fields)...);
			}(std::make_index_sequence<sizeof...(T)>());
	}

	template <typename... T, typename C>
	constexpr void IndexedTupleFor(std::tuple<T...>& Fields, C&& Callable)
	{
		[&]<auto... Idx>(std::index_sequence<Idx...>){
				(void)(Callable(Idx, std::get<Idx>(Fields)),...);
			}(std::make_index_sequence<sizeof...(T)>());
	}

	template <auto T>
	static constexpr auto NameOfField = detail::NameOfFieldImpl<T>::Get();

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#endif

	template <class T>
	struct Wrapper {
		using Type = T;
		T v;
	};

	template <class T>
	Wrapper(T) -> Wrapper<T>;

    // This avoids clang triggerings an error about using addresses to fields (it says it cannot use the address of subobject, but it should)
	template <class T>
	constexpr auto Wrap(const T& arg) noexcept {
		return Wrapper{arg};
	}

	template <typename T>
	consteval auto GetFieldNames()
	{
		constexpr auto Fields = ToTuple(detail::SampleObject<T>);
		return [&]<auto... Idx>(std::index_sequence<Idx...>)
			{
				return std::array{NameOfField<Wrap(&std::get<Idx>(Fields))>.c_str()...};
			}(std::make_index_sequence<std::tuple_size_v<decltype(Fields)>>());
	}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

}