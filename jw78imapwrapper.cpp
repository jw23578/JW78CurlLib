#include "jw78imapwrapper.h"
#include "jw78curlwrapper.h"

#ifdef JW78_WITH_IMAP

#include "utils/extstring.h"
#include "utils/extvector.h"

jw78::IMAPWrapper::IMAPWrapper(const std::string &host,
                               const int port,
                               const std::string &user,
                               const std::string &password):
    host(host),
    port(port),
    user(user),
    password(password)
{
    url = "imaps://";
    url += host + ":" + ExtString::toString(port);
}

bool jw78::IMAPWrapper::fetchFolder(std::string &errorMessage)
{
    CurlWrapper curl;
    curl.setUserAndPassword(user, password);
    std::string result;
    if (!curl.get(url, result, errorMessage))
    {
        return false;
    }
    std::vector<std::string> lines;
    ExtString::split(result, "\r\n", lines);
    IMAPFolders tempFolder;
    for (size_t i(0); i < lines.size(); ++i)
    {
        std::string l(ExtString::trim(lines[i]));
        std::string attributes(ExtString::between(l, "(", ")", false));
        IMAPFolder *f(new IMAPFolder);
        tempFolder.insert(f);
        ExtString::split(attributes, " ", f->attributes);
        l = ExtString::rightOf(l, "\"/\" ");
        l = ExtString::unQuote(l, "\"", "\\");
        ExtString::split(l, "/", f->name);
    }
    IMAPFolders::iterator it(tempFolder.begin());
    while (it != tempFolder.end())
    {
        IMAPFolder *f(*it);
        if (f->name.size() == 1)
        {
            allFolders.insert(f);
            it = tempFolder.erase(it);
        }
        else
        {
            ++it;
        }
    }
    while (tempFolder.size())
    {
        IMAPFolders::iterator it(tempFolder.begin());
        while (it != tempFolder.end())
        {
            IMAPFolders::iterator allIt(allFolders.begin());
            bool added(false);
            while (allIt != allFolders.end())
            {
                if ((*allIt)->addFolder(*it))
                {
                    it = tempFolder.erase(it);
                    added = true;
                    break;
                }
                ++allIt;
            }
            if (!added)
            {
                ++it;
            }
        }
    }
    return true;
}
#include <iostream>

bool jw78::IMAPWrapper::updateFolder(jw78::IMAPWrapper::IMAPFolder &folder)
{
    CurlWrapper curl;
    curl.setUserAndPassword(user, password);
    std::string result;
    std::string request("STATUS ");
    request += folder.getFullName() + " (MESSAGES RECENT UNSEEN)";
    std::string errorMessage;
    if (!curl.customRequest(url, request, result, errorMessage))
    {
        return false;
    }
    std::vector<std::string> status;
    ExtString::splitBetween(result, "(", ")", false, " ", status);
    for (size_t i(0); i < status.size(); ++i)
    {
        std::string &s(status[i]);
        if (s == "MESSAGES")
        {
            ExtString::to(status[i + 1], folder.messages);
        }
        if (s == "RECENT")
        {
            ExtString::to(status[i + 1], folder.recent);
        }
        if (s == "UNSEEN")
        {
            ExtString::to(status[i + 1], folder.unseen);
        }
    }

    request = "FETCH 1:* UID";
    std::string myUrl(url);
    myUrl += std::string("/") + folder.getFullName();
    if (!curl.customRequest(myUrl, request, result, errorMessage))
    {
        return false;
    }
    std::vector<std::string> lines;
    ExtString::split(result, "\r\n", lines);
    for (size_t i(0); i < lines.size(); ++i)
    {
        std::vector<std::string> data;
        ExtString::splitBetween(lines[i], "(", ")", false, " ", data);
        folder.emails.push_back(new EMailWrapper(data[1], ""));
        std::cout << data[1] << std::endl;
    }
    return true;
}

bool jw78::IMAPWrapper::fetchEMail(const std::string &uid)
{
    CurlWrapper curl;
    curl.setUserAndPassword(user, password);
    std::string myUrl(url);
    myUrl += "/inbox/;uid=";
    myUrl += uid;
    std::string result;
    std::string errorMessage;
    curl.get(myUrl, result, errorMessage);
    std::cout << result << std::endl;
    std::cout << errorMessage << std::endl;
}

bool jw78::IMAPWrapper::fetchHeader(const std::string &uid,
                                    std::string &header)
{
    CurlWrapper curl;
    curl.setUserAndPassword(user, password);
    std::string myUrl(url);
    myUrl += "/inbox/;uid=";
    myUrl += uid;
    myUrl += ";section=HEADER";
    std::string errorMessage;
    curl.get(myUrl, header, errorMessage);
    std::cout << errorMessage << std::endl;
}

bool jw78::IMAPWrapper::fetchText(const std::string &uid,
                                    std::string &text)
{
    CurlWrapper curl;
    curl.setUserAndPassword(user, password);
    std::string myUrl(url);
    myUrl += "/inbox/;uid=";
    myUrl += uid;
    myUrl += ";section=TEXT";
    std::string errorMessage;
    curl.get(myUrl, text, errorMessage);
    std::cout << errorMessage << std::endl;
}

jw78::IMAPWrapper::IMAPFolder::IMAPFolder():
    parent(nullptr),
    messages(0),
    recent(0),
    unseen(0)
{
}

bool jw78::IMAPWrapper::IMAPFolder::addFolder(jw78::IMAPWrapper::IMAPFolder *newFolder)
{
    if (getName() != newFolder->getNamePart(name.size() - 1))
    {
        return false;
    }
    if (name.size() + 1 == newFolder->name.size())
    {
        newFolder->parent = this;
        subFolder.insert(newFolder);
        return true;
    }
    for (auto f: subFolder)
    {
        if (f->addFolder(newFolder))
        {
            return true;
        }
    }
    return false;
}

const std::string &jw78::IMAPWrapper::IMAPFolder::getNamePart(int index)
{
    return name[index];
}

const std::string &jw78::IMAPWrapper::IMAPFolder::getName()
{
    return getNamePart(name.size() - 1);
}

std::string jw78::IMAPWrapper::IMAPFolder::getFullName()
{
    return ExtVector::concat(name, "/", "", "");
}

std::string jw78::IMAPWrapper::IMAPFolder::getInfoString()
{
    return getFullName() + " Messages: " + ExtString::toString(messages) + " Unseen:" + ExtString::toString(unseen) + " Recent: " + ExtString::toString(recent);
}

#endif // JW78_WITH_IMAP
