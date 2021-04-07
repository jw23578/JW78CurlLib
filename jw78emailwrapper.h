#ifndef JW78EMAILWRAPPER_H
#define JW78EMAILWRAPPER_H

#include <string_view>
#include <string>

namespace jw78 {

class EMailWrapper
{
    std::string uid;
    std::string_view completeContent;
    bool parsed = {false};
    void parse();
    std::string_view fromLine;
    std::string_view toLine;
    std::string_view subjectLine;
    std::string_view dateLine;
    std::string_view messageIdLine;
public:
    EMailWrapper(std::string const &uid,
                 std::string const &c);
    void saveToFile(std::string const &filename);
    std::string_view getFromLine();
    std::string_view getMessageIdLine();
};

}
#endif // JW78EMAILWRAPPER_H
