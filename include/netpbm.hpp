#include <fstream>
#include <vector>
#include <stdexcept>
using namespace std;

namespace ash {

struct pixel_t {
    unsigned char r, g, b;

    unsigned char& operator[](int i) {
        unsigned char* arr = &r;
        return arr[i];
    }

    unsigned char operator[](int i) const {
        const unsigned char* arr = &r;
        return arr[i];
    }
};

pixel_t k_black = {0, 0, 0};

std::ostream& operator<<(std::ostream& out, const pixel_t& pixel) {
    return out << '('
                         << static_cast<int>(pixel.r) << ", "
                         << static_cast<int>(pixel.g) << ", "
                         << static_cast<int>(pixel.b) << ')';
}

namespace detail {

std::string next_token(std::istream& in) {
    std::string token;
    // get first non-space out-of-comment character
    in >> std::ws;
    while (in && in.peek() == '#') {
        in.ignore(numeric_limits<streamsize>::max(), '\n');
        in >> std::ws;
    }
    // keep reading characters, ignoring comments, until a whitespace
    // is encountered
    char c;
    while (in.get(c) && !std::isspace(c)) {
        if (c == '#')
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        else
            token += c;
    }
    return token;
}

int next_int(std::istream& in) {
    std::string token = next_token(in);
    size_t processed;
    int value = std::stoi(token, &processed);
    if (processed != token.size())
        throw std::invalid_argument(
                "Cannot convert '" + token + "' to integer");
    return value;
}

}

class img_t {
    public:
        static img_t load(const std::string& filename) {
            std::ifstream in(filename);
            if (!in)
                throw std::invalid_argument("Couldn't open " + filename);
            std::string magic(2,' ');
            if (!in.get(magic[0]) || !in.get(magic[1]) || !(magic=="P3" || magic=="P6"))
                throw std::invalid_argument(
                        "File " + filename + " is not a proper netpbm file");
            size_t width = detail::next_int(in);
            size_t height = detail::next_int(in);
            size_t depth = detail::next_int(in);
            if (depth != 255)
                throw std::runtime_error(
                        "Depth of " + std::to_string(depth) + " not allowed");
            img_t image(width, height);
            if (magic == "P3")
                image.load_ascii(in);
            else // if (magic == "P6")
                image.load_binary(in);
            return image;
        }

        void save(const std::string& filename, bool binary = true) {
            auto mode = ios::out;
            if (binary)
                mode = mode | std::ios::binary;
            std::ofstream out(filename, mode);
            if (!out)
                throw std::invalid_argument("Couldn't open " + filename);
            out << 'P' << (binary? '6' : '3') << ' '
                    << m_width << ' ' << m_height << " 255\n";
            if (binary) {
                for (const auto& pixel : m_data)
                    out << pixel.r << pixel.g << pixel.b;
            }
            else
                for (const auto& pixel : m_data)
                    out << ((int)pixel.r) << ' '
                        << ((int)pixel.g) << ' '
                        << ((int)pixel.b) << '\n';
        }

        img_t(size_t width, size_t height, const pixel_t& color=k_black) :
            m_width(width), m_height(height), m_data(width*height, color) {
        }

        const pixel_t& operator()(int x, int y) const {
            int index = (m_height-y-1)*m_width + x;
            return m_data[index];
        }

        pixel_t& operator()(int x, int y) {
            return const_cast<pixel_t&>(
                    static_cast<const img_t&>(*this)(x, y));
        }

        size_t width() const {
            return m_width;
        }

        size_t height() const {
            return m_height;
        }

    private:

        void load_binary(std::ifstream& in) {
            size_t expected_bytes = m_data.size()*sizeof(pixel_t);
            in.read(reinterpret_cast<char*>(m_data.data()), expected_bytes);
            size_t read = in.gcount();
            if (read != expected_bytes) {
                throw runtime_error("Expected " + to_string(expected_bytes) +
                        " bytes, received " + to_string(read) + " instead");
            }
        }

        void load_ascii(std::ifstream& in) {
            for (pixel_t& pixel : m_data) {
                pixel.r = detail::next_int(in);
                pixel.g = detail::next_int(in);
                pixel.b = detail::next_int(in);
            }
        }
 
        size_t m_width, m_height;
        std::vector<pixel_t> m_data;
};

} // namespace ash

