// #pragma once

// #include <cstdint>
// #include <type_traits>

// struct Percent {
//     template<typename num_t>
//     Percent(num_t num) {}

//     template<typename num_t>
//     void set(num_t num) {
//         static_assert(std::is_arithmetic<num_t>::value, "Num type must be numeric");
//         if(std::is_signed<num_t>::value) {
//             bits = (0b1 << 8 & num ? )
//         } else {
//             bits = (num & m_64);    // first 6 bits, range up to 64
//         }
//     }

// private:
//     static const uint16_t
//         m_64 = 0b111111,
//         m_16 = 0b1111,
//         m_4 = 0b11;

//     //static const uint16_t test = m_1 | m_2 | m_3 | m_4;

//     uint16_t bits;
// };