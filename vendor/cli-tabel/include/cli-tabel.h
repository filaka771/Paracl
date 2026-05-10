#include <iostream>
#include <vector>
#include <tuple>
#include <cstdint>
#include <utility>
#include <string>
#include <sstream>
#include <type_traits>

// Streamable detection
template<typename, typename = void>
struct is_streamable : std::false_type {};

template<typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostringstream&>() << std::declval<T>())>>
    : std::true_type {};

template<typename T>
inline constexpr bool is_streamable_v = is_streamable<T>::value;

template<typename T>
struct field_tuple;


template<typename T>
class CliTable {
public:
    void print(const std::vector<T>& rows_vec,
               const std::string& separator = " | ",
               bool precise_alignment = false) const {
        constexpr auto fields = field_tuple<T>::value;
        uint32_t col_num = std::tuple_size<decltype(fields)>::value;

        std::vector<uint32_t> alignments(col_num, 0);

        if (precise_alignment)
            calculate_alignments(rows_vec, alignments);

        for(const auto& item: rows_vec) {
            print_row(item, separator, alignments);
        }
    }

private:
    void print_row(const T& item, const std::string& sep,
                   std::vector<uint32_t>& alignments) const {
        constexpr auto fields = field_tuple<T>::value;

        uint32_t col = 0;

        std::apply([&](auto... member_ptr) {
            ((print_field(col, alignments, sep, item.*member_ptr)), ...);
        }, fields);
        std::cout << std::endl;
    }

    template<typename Field>
    void print_field(uint32_t& col, std::vector<uint32_t>& alignments,
                     const std::string& sep, const Field& field) const {
        std::string str_field = convert_to_string(field);
        uint32_t field_length = str_field.size();
        if(field_length > alignments[col])
            alignments[col] = field_length;

        if(col)
            std::cout << sep;
        std::cout << str_field << std::string(alignments[col] - field_length, ' ');
        col ++;
    }

    // Convert to string
    template<typename Field> 
    std::string convert_to_string(const Field& value) const {
        if constexpr (std::is_arithmetic_v<Field>)
            return std::to_string(value);

        else if constexpr (std::is_enum_v<Field>)
            return std::to_string(static_cast<std::underlying_type_t<Field>>(value));

        else if constexpr (is_streamable_v<Field>) {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
        else
            static_assert(sizeof(Field) == 0, "Type is not printable"
                                              "(no operator<< and not arithmetic/enum)");
    }

    void calculate_alignments(const std::vector<T>& rows_vec,
                            std::vector<uint32_t>& alignments) const {
        constexpr auto fields = field_tuple<T>::value;
        // Initialize alignments to zeros (size must be set before calling)
        alignments.assign(std::tuple_size_v<decltype(fields)>, 0);

        for (const auto& item : rows_vec) {
            uint32_t col = 0;
            std::apply([&](auto... member_ptr) {
                ((update_width(col, alignments, item.*member_ptr)), ...);
            }, fields);
        }
    }

    template<typename Field>
    void update_width(uint32_t& col, std::vector<uint32_t>& widths, const Field& field) const {
        std::string str = convert_to_string(field);
        if (str.size() > widths[col])
            widths[col] = str.size();
        ++col;
    }
};

struct Person {
    std::string name;
    int age;
    double height;  // in meters
};

// Specialize field_tuple for Person – list the member pointers in the desired column order
template<>
struct field_tuple<Person> {
    static constexpr auto value = std::make_tuple(
        &Person::name,
        &Person::age,
        &Person::height
    );
};

int main() {
    std::vector<Person> people = {
        {"Alice", 30, 1.65},
        {"Bob", 25, 1.80},
        {"Charlie", 35, 1.75}
    };

    CliTable<Person> table;
    table.print(people, " | ");
    std::cout << std::endl;
    table.print(people, " | ", /*precise_alignment=*/ true);

    return 0;
}
