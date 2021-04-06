#ifndef JW78CURLWRAPPER_H
#define JW78CURLWRAPPER_H

#include <curl/curl.h>
#include <string>
#include <fstream>
#include <vector>

namespace jw78 {

class CurlWrapper
{
    friend size_t myWriteFunction(void *contents, size_t size, size_t nmemb, void *userp);
    friend int myDebugFunction(CURL *handle,
                               curl_infotype type,
                               char *data,
                               size_t size,
                               void *userptr);
    std::string payload;
    size_t payloadPosition;
    friend size_t payloadSource(void *ptr, size_t size, size_t nmemb, void *userp);

    CURL *curl;

    struct sTarget
    {
        std::string *stringTarget;
        std::ostream *streamTarget;
        std::string stringDebugTarget;
        sTarget():stringTarget(nullptr), streamTarget(nullptr) {}
        sTarget(std::string &s):stringTarget(&s), streamTarget(nullptr) {}
        sTarget(std::ostream &s):stringTarget(nullptr), streamTarget(&s) {}
        void add(char *data, size_t size)
        {
            if (stringTarget)
            {
                stringTarget->append(data, size);
                return;
            }
            if (streamTarget)
            {
                streamTarget->write(data, size);
                return;
            }
        }
        void addDebug(char *data, size_t size)
        {
            stringDebugTarget += std::string(data, size);
        }
    };

    void setOpt(CURLoption option, long i);
    void setOpt(CURLoption option, std::string const &s);

public:
    static size_t globalInitCount;
    CurlWrapper();
    ~CurlWrapper();

    void setUserAndPassword(std::string const &user,
                            std::string const &password);

    bool pop3NumberAndSizes(std::string const &host,
                            std::string const &user,
                            std::string const &password,
                            std::string &target);
    bool pop3Retrieve(std::string const &host,
                      std::string const &user,
                      std::string const &password,
                      const int number,
                      std::string &target);
    bool pop3Delete(std::string const &host,
                    std::string const &user,
                    std::string const &password,
                    const int number);
    void smtpSendMail(std::string const &host,
                      std::string const &user,
                      std::string const &password,
                      std::string const &mailFrom,
                      std::vector<std::string> toAddr,
                      std::vector<std::string> ccAddr,
                      std::vector<std::string> bccAddr,
                      std::string const &emailPayload,
                      std::string &logResult);

    bool customRequest(const std::string &url,
                       const std::string &request,
                       std::string &result,
                       std::string &message);
    bool get(std::string const &url,
             std::string &result,
             std::string &message);

    bool httpsPost(std::string const &url,
                   const std::string &postData,
                   std::string &result,
                   std::string &message);

    CURLcode curlPerformToString(std::string &result);
};

}


#endif // JW78CURLWRAPPER_H
