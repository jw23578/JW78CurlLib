#ifndef JW78POP3WRAPPER_H
#define JW78POP3WRAPPER_H

#include "jw78curlwrapper.h"
#include <string>
#include <vector>


class jw78Pop3Wrapper
{
    std::string host;
    std::string user;
    std::string password;
    jw78CurlWrapper cw;
public:
    jw78Pop3Wrapper(std::string const &h,
                    std::string const &u,
                    std::string const &pwd);

    bool retrieveEMail(int number,
                       std::string &target);
    bool deleteEMail(int number);

    void eMailSizes(std::vector<size_t> &sizes);

};

#endif // JW78POP3WRAPPER_H
