#pragma once

#include <type_traits>
#include <atomic>
#include <deque>


template<
	template<typename...> typename,
	template<typename...> typename>
struct is_same_template : std::false_type{};
template<
	template<typename...> typename T>
struct is_same_template<T, T> : std::true_type{};

template<
	typename, typename>
struct is_same_template_base : std::false_type{};
template<
	template<typename...> typename T,
	typename... A, typename... B>
struct is_same_template_base<T<A...>, T<B...>> : std::true_type{};

template<
	template<typename...> typename, typename>
struct is_same_template_mixed : std::false_type{};
template<
	template<typename...> typename T, typename... A>
struct is_same_template_mixed<T, T<A...>> : std::true_type{};


template<template<typename...> typename T, template<typename...> typename... Ts>
using any_same_template = std::disjunction<is_same_template<T, Ts>...>;


template<
	template<typename...> class C, typename...Ts
> std::true_type is_base_of_template_impl(const C<Ts...>*);
template<
	template<typename...> class C
> std::false_type is_base_of_template_impl(...);
template<
	typename T, template<typename...> class C
> using is_base_of_template = decltype(is_base_of_template_impl<C>(std::declval<T*>()));	// assert inheritance of template class

template
	<template <size_t> class C, size_t S
> std::true_type is_base_of_num_template_impl(const C<S>*);
template<
	template <size_t...> class C
> std::false_type is_base_of_num_template_impl(...);
template<
	typename T, template <size_t> class C
> using is_base_of_num_template = decltype(is_base_of_num_template_impl<C>(std::declval<T*>()));	// assert inheritance of size templated class



template<class derived_t>
class Instanced {
	typedef struct Instanced<derived_t>	This_t;
public:
	inline explicit Instanced() : instance(This_t::deleted.empty() ? This_t::highest + 1 : This_t::deleted.front()) {
		static_assert(std::is_base_of<This_t, derived_t>::value, "Template paramter (derived_t) must inherit from Instanced<derived_t> for CRTP.");
		if(This_t::deleted.empty()) {
			This_t::highest++;
		} else {
			This_t::deleted.pop_front();
		}
	}
	inline virtual ~Instanced() {
		This_t::deleted.push_back(this->instance);
	}

	inline uint32_t getInst() const { return this->instance; }

	inline static uint32_t getInstances() { return This_t::highest - This_t::deleted.size(); }
	template<class type>
	inline static uint32_t getInstances() {
		static_assert(std::is_base_of<Instanced<type>, type>::value, "Template parameter (type) must inherit from Instanced<type>.");
		return Instanced<type>::getInstances();
	}

protected:
	const uint32_t instance;

private:
	static inline std::atomic<uint32_t> highest{0};
	static inline std::deque<uint32_t> deleted;


};