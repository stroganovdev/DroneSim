
#pragma once

#include "Misc/TVariant.h"
#include <type_traits>

template <class> inline constexpr bool AlwaysFalseV = false;
template <class... Fs>
struct TVariantMatch : Fs... { using Fs::operator()...; };
template <class... Fs>
TVariantMatch(Fs...) -> TVariantMatch<Fs...>;

template <typename T>
struct TDroneIsVariant : std::false_type {};

template <typename T, typename... Ts>
struct TDroneIsVariant<TVariant<T, Ts...>> : std::true_type {};

template <typename T>
concept UEVariant = TDroneIsVariant<std::decay_t<T>>::value;


/**
 * Matches the provided variant with one of the functions.
 * If all variant types are not handled, this generates a compile error.
 *
 * Example:
 * \code
 * using MyVariant = TVariant<A, B>;
 * const MyVariant& x = ...;
 *
* // Compiles
 * match_variant(
 *	   x,
 *     (const A& a) {},
 *     (const B& b) {}
 * );
 *
 * // Does not compile, missing variant
 * match_variant(
 *	   x,
 *     (const A& a) {}
 * );
 * 
 * \endcode
 * 
 * @tparam Fs 
*/
template <typename VariantT, typename... Fs>
requires UEVariant<VariantT>
FORCEINLINE decltype(auto) match_variant(VariantT&& Variant, Fs&&... Arms)
{
	return Visit(
		TVariantMatch{ Forward<Fs>(Arms)... },
		Forward<VariantT>(Variant)
	);
}

// Convenience: pointer variant
template <typename VariantT, typename... Fs>
requires UEVariant<VariantT>
FORCEINLINE decltype(auto) match_variant(const VariantT* VariantPtr, Fs&&... Arms)
{
	check(VariantPtr);
	return match_variant(*VariantPtr, Forward<Fs>(Arms)...);
}
