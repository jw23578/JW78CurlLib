#include "jw78curlwrapper.h"
#include <iostream>
#include <sstream>
#include <cstring>

size_t jw78CurlWrapper::globalInitCount(0);

size_t myWriteFunction(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    jw78CurlWrapper::sTarget *target(static_cast<jw78CurlWrapper::sTarget*>(userp));
    target->add(static_cast<char*>(contents), realsize);
    return realsize;
}

int myDebugFunction(CURL *handle,
                   curl_infotype type,
                   char *data,
                   size_t size,
                   void *userptr)
{
    jw78CurlWrapper::sTarget *target(static_cast<jw78CurlWrapper::sTarget*>(userptr));
    target->addDebug(data, size);
    return 0;
}

size_t payloadSource(void *ptr, size_t size, size_t nmemb, void *userp)
{
    if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1))
    {
        return 0;
    }
    jw78CurlWrapper *curlWrapper((jw78CurlWrapper*)userp);
    if (curlWrapper->payloadPosition >= curlWrapper->payload.size())
    {
        return 0;
    }
    size_t len(size * nmemb);
    len = std::min(len, curlWrapper->payload.size() - curlWrapper->payloadPosition);
    std::memcpy(ptr,
                (const void*)&(curlWrapper->payload.data()[curlWrapper->payloadPosition]),
            len);
    curlWrapper->payloadPosition += len;
    return len;
}

void jw78CurlWrapper::setOpt(CURLoption option, long i)
{
    curl_easy_setopt(curl, option, i);
}

void jw78CurlWrapper::setOpt(CURLoption option, const std::string &s)
{
    curl_easy_setopt(curl, option, s.c_str());
}

jw78CurlWrapper::jw78CurlWrapper():
    curl(nullptr)
{
    if (globalInitCount == 0)
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }
    curl_version_info_data *version(curl_version_info(CURLVERSION_NOW));
    globalInitCount += 1;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, myWriteFunction);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, myDebugFunction);
    setOpt(CURLOPT_SSL_VERIFYPEER, 0L);
    setOpt(CURLOPT_SSL_VERIFYHOST, 0L);
    setOpt(CURLOPT_VERBOSE, 1L);
}

jw78CurlWrapper::~jw78CurlWrapper()
{
    curl_easy_cleanup(curl);
    globalInitCount -= 1;
    if (globalInitCount == 0)
    {
        curl_global_cleanup();
    }
}

void jw78CurlWrapper::setUserAndPassword(const std::string &user, const std::string &password)
{
    setOpt(CURLOPT_USERNAME, user);
    setOpt(CURLOPT_PASSWORD, password);
}

bool jw78CurlWrapper::pop3NumberAndSizes(const std::string &host,
                                         const std::string &user,
                                         const std::string &password,
                                         std::string &target)
{
    setUserAndPassword(user, password);
    std::string url("pop3s://");
    url += host + "/";
    setOpt(CURLOPT_URL, url);
    CURLcode res(curlPerformToString(target));
    return res == CURLE_OK;
}



bool jw78CurlWrapper::pop3Retrieve(const std::string &host,
                                   const std::string &user,
                                   const std::string &password,
                                   const int number,
                                   std::string &target)
{
    setUserAndPassword(user, password);
    std::stringstream ss;
    ss << "pop3s://";
    ss << host;
    ss << "/";
    ss << number;
    setOpt(CURLOPT_URL, ss.str());
    CURLcode res(curlPerformToString(target));
    return res == CURLE_OK;
}

bool jw78CurlWrapper::pop3Delete(const std::string &host,
                                 const std::string &user,
                                 const std::string &password,
                                 const int number)
{
    setUserAndPassword(user, password);
    std::stringstream ss;
    ss << "pop3s://";
    ss << host;
    ss << "/";
    ss << number;
    setOpt(CURLOPT_URL, ss.str());
    std::string request("DELE");
    setOpt(CURLOPT_CUSTOMREQUEST, request);
    setOpt(CURLOPT_NOBODY, 1L);
    std::string result;
    CURLcode res(curlPerformToString(result));
    return res == CURLE_OK;
}

bool jw78CurlWrapper::imapFetchFolderList(const std::string &host, const std::string &user, const std::string &password, const std::string &folder)
{
    setUserAndPassword(user, password);
    std::string url("imaps://");
    url += host;
    url += folder;
    setOpt(CURLOPT_URL, url);
    std::string result;
    CURLcode res(curlPerformToString(result));
    return res == CURLE_OK;
}

void jw78CurlWrapper::smtpSendMail(const std::string &host,
                                   const std::string &user,
                                   const std::string &password,
                                   const std::string &mailFrom,
                                   std::vector<std::string> toAddr,
                                   std::vector<std::string> ccAddr,
                                   std::vector<std::string> bccAddr,
                                   const std::string &emailPayload,
                                   std::string &logResult)
{
    setOpt(CURLOPT_USERNAME, user);
    setOpt(CURLOPT_PASSWORD, password);
    setOpt(CURLOPT_URL, host);
    setOpt(CURLOPT_MAIL_FROM, mailFrom);
    curl_slist *recipients(0);
    for (size_t i(0); i < toAddr.size(); ++i)
    {
        recipients = curl_slist_append(recipients, toAddr[i].data());
    }
    for (size_t i(0); i < ccAddr.size(); ++i)
    {
        recipients = curl_slist_append(recipients, ccAddr[i].data());
    }
    for (size_t i(0); i < bccAddr.size(); ++i)
    {
        recipients = curl_slist_append(recipients, bccAddr[i].data());
    }
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
    payload = emailPayload;
    payloadPosition = 0;
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payloadSource);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    sTarget target;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&target);
    curl_easy_setopt(curl, CURLOPT_DEBUGDATA, (void *)&target);
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, myDebugFunction);
    CURLcode res(curl_easy_perform(curl));
    curl_slist_free_all(recipients);
    logResult = target.stringDebugTarget;
}

bool jw78CurlWrapper::get(const std::string &url, std::string &result, std::string &message)
{
    setOpt(CURLOPT_URL, url);
    CURLcode res(curlPerformToString(result));
    return res == CURLE_OK;
}

bool jw78CurlWrapper::httpsPost(const std::string &url,
                                const std::string &postData,
                                std::string &result,
                                std::string &message)
{
    setOpt(CURLOPT_POSTFIELDS, postData);
    setOpt(CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    CURLcode res(curlPerformToString(result));
    return res == CURLE_OK;
}

CURLcode jw78CurlWrapper::curlPerformToString(std::string &result)
{
    sTarget target(result);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&target);
    curl_easy_setopt(curl, CURLOPT_DEBUGDATA, (void *)&target);
    return curl_easy_perform(curl);
}
