#pragma once

#include <string>
#include <vector>

class Serializer
{
public:
    using ser_data_t = std::vector<char>;

    virtual ser_data_t Serialize() const             = 0;
    virtual void       Deserialize(ser_data_t& data) = 0;

protected:
    static inline const std::string Delim{","};

    template <typename T>
    static void insert(ser_data_t& data, ser_data_t::iterator it, T const& in, std::string const& delim = Delim)
    {
        if constexpr (std::is_integral_v<T>) {
            auto str = std::to_string(in) + delim;
            data.insert(it, str.begin(), str.end());
        } else if constexpr (std::is_same_v<std::decay_t<T>, char*>) {
            // char array
            auto str = std::string(in) + delim;
            data.insert(it, str.begin(), str.end());
        } else if constexpr (std::is_same_v<T, std::string>) {
            auto str = in + delim;
            data.insert(it, str.begin(), str.end());
        } else if constexpr (std::is_same_v<T, ser_data_t>) {
            data.insert(it, in.begin(), in.end());
        } else {
            std::string str = "Serialization not supported for type: " + std::string(typeid(T).name());
            throw std::runtime_error(str);
        }
    }

    template <typename T>
    static void append(ser_data_t& data, T const& in, std::string const& delim = Delim)
    {
        insert(data, data.end(), in, delim);
    }
};
