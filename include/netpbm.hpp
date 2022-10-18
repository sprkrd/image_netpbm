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

std::string next_token(std::istream& in) {
  std::string token;
  // get first non-space out-of-comment character
  char c;
  in.get(c);
  while (in && isspace(c)) {
    if (c == '#')
      in.ignore(numeric_limits<streamsize>::max(), '\n');
    in.get(c);
  }
  // keep reading characters, ignoring comments, until a whitespace
  // is encountered
  while (in && !std::isspace(c)) {
    if (c == '#')
      in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    else
      token += c;
    in.get(c);
  }
  return token;
}

int to_int(const std::string& token) {
  size_t processed;
  int value = std::stoi(token, &processed);
  if (processed != token.size())
    throw std::invalid_argument(
        "Cannot convert '" + token + "' to integer");
  return value;
}

int next_int(std::istream& in) {
  std::string token = next_token(in);
  return to_int(token);
}

std::istream& get_next_byte(std::istream& in, int& b) {
  std::string token = next_token(in);
  if (!token.empty()) {
    b = to_int(token);
    if (b < 0 || b > 255)
      throw std::out_of_range(
          "Byte " + std::to_string(b) + " not in range [0,255]");
  }
  return in;
}

std::istream& get_next_byte(std::istream& in, char& b) {
  return in.get(b);
}

class img_t {
  public:
    static img_t load(const std::string& filename) {
      std::ifstream in(filename);
      if (!in)
        throw std::invalid_argument("Couldn't open " + filename);
      std::string magic(2,' ');
      in.get(magic[0]);
      in.get(magic[1]);
      if (!in || (magic!="P3"&&magic!="P6"))
        throw std::invalid_argument(
            "File " + filename + " is not a proper netbpm file");
      size_t width = next_int(in);
      size_t height = next_int(in);
      size_t depth = next_int(in);
      if (depth != 255)
        throw std::runtime_error(
            "Depth of " + std::to_string(depth) + " not allowed");
      img_t image(width, height);
      if (magic == "P3")
        image.load_data<int>(in);
      else // if (magic == "P6")
        image.load_data<char>(in);
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

    template <class byte_t>
    void load_data(std::ifstream& in) {
      size_t expected_bytes = m_data.size()*3;
      size_t read_bytes = 0;
      int index = 0;
      int channel = 0;
      pixel_t next_pixel;
      byte_t b;
      while (read_bytes < expected_bytes && get_next_byte(in, b)) {
        ++read_bytes;
        next_pixel[channel] = static_cast<unsigned char>(b);
        if (channel == 2) {
          m_data[index] = next_pixel;
          ++index;
          channel = 0;
        }
        else
          ++channel;
      }
      if (read_bytes != expected_bytes)
        throw runtime_error("Expected " + to_string(expected_bytes) +
            " bytes, received " + to_string(read_bytes) + " instead");
    }
 
    size_t m_width, m_height;
    std::vector<pixel_t> m_data;
};

} // namespace ash

namespace std {

template<>
struct hash<ash::pixel_t> {
  size_t operator()(const ash::pixel_t& pixel) const {
    size_t result = pixel.r;
    result = (result<<8)|pixel.g;
    result = (result<<8)|pixel.b;
    return result;
  }
};

}

