#ifndef JW78POP3WRAPPER_H
#define JW78POP3WRAPPER_H

// Ein Kommentar

#ifdef JW78_WITH_POP3

#include "jw78curlwrapper.h"
#include <string>
#include <vector>

namespace jw78 {

class Pop3Wrapper
{
    std::string host;
    std::string user;
    std::string password;
    CurlWrapper cw;
public:
    Pop3Wrapper(std::string const &h,
                std::string const &u,
                std::string const &pwd);

    bool retrieveEMail(int number,
                       std::string &target);
    bool deleteEMail(int number);

    void eMailSizes(std::vector<size_t> &sizes);

};

}

#endif // JW78_WITH_POP3

#endif // JW78POP3WRAPPER_H
