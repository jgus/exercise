
#pragma once

#include <string>

namespace obfuscated
{
    template<typename TChar, std::size_t N>
    struct obfuscated_string_hash_helper
    {
        constexpr static typename std::make_unsigned<TChar>::type hash(const char* s)
        {
            return (obfuscated_string_hash_helper<TChar, N - 1>::hash(s) << 5) ^
                   (obfuscated_string_hash_helper<TChar, N - 1>::hash(s) >> 3) ^
                   obfuscated_string_hash_helper<TChar, N - 1>::hash(s) ^
                   s[N - 1];
        }
    };

    template<typename TChar>
    struct obfuscated_string_hash_helper<TChar, 0>
    {
        constexpr static typename std::make_unsigned<TChar>::type hash(const char* s)
        {
            return 0;
        }
    };

    template<typename TChar, std::size_t N>
    class obfuscated_string
    {
    public:
        constexpr explicit obfuscated_string(const TChar* s) : tail(encode(s[N - 1])), head(s) {};

        std::basic_string<TChar> dump() const { return head.dump() + tail; }

        std::basic_string<TChar> string() const { return head.string() + decode(tail); }

        operator std::basic_string<TChar>() const { return string(); }

    private:
        TChar                           tail;
        obfuscated_string<TChar, N - 1> head;

        template<std::size_t M>
        static constexpr typename std::make_unsigned<TChar>::type hash(const char(& s)[M])
        {
            return obfuscated_string_hash_helper<TChar, M - 1>::hash(s);
        }

        static constexpr typename std::make_unsigned<TChar>::type salt = typename std::make_unsigned<TChar>::type{ 0x80 | (N + hash(__TIME__)) };

        static constexpr TChar encode(TChar c) { return c + salt; }

        static constexpr TChar decode(TChar c) { return c - salt; }
    };

    template<typename TChar>
    class obfuscated_string<TChar, 0>
    {
    public:
        constexpr explicit obfuscated_string(const TChar* s) {};

        std::basic_string<TChar> dump() const { return std::basic_string<TChar>(); }

        std::basic_string<TChar> string() const { return std::basic_string<TChar>(); }

        operator std::basic_string<TChar>() const { return string(); }
    };

    template<typename TChar, std::size_t N>
    inline constexpr obfuscated_string<TChar, N - 1> obfuscate(const TChar(& plaintext)[N])
    {
        return obfuscated_string<TChar, N - 1>(plaintext);
    }
}
