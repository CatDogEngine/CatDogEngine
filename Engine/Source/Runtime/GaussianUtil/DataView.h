#include <vector>

class DataView
{
public:
    DataView(const std::vector<std::byte>& buffer, size_t offset)
        : buffer(buffer), offset(offset)
    {
    }

    template <typename T>
    T get(size_t index) const
    {
        T value;
        std::memcpy(&value, &buffer[offset + index], sizeof(T));
        return value;
    }

private:
    const std::vector<std::byte>& buffer;
    size_t offset;
};