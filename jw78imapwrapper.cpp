#include "jw78imapwrapper.h"
#include "jw78curlwrapper.h"
#include "extstring.h"
#include "extvector.h"

jw78::IMAPWrapper::IMAPWrapper(const std::string &host,
                               const int port,
                               const std::string &user,
                               const std::string &password):
    host(host),
    port(port),
    user(user),
    password(password)
{

}

#include <iostream>

bool jw78::IMAPWrapper::fetchFolder(std::string &errorMessage)
{
    CurlWrapper curl;
    curl.setUserAndPassword(user, password);
    std::string url("imaps://");
    url += host + ":" + extString::toString(port);
    std::string result;
    if (!curl.get(url, result, errorMessage))
    {
        return false;
    }
    std::vector<std::string> lines;
    extString::split(result, "\r\n", lines);
    IMAPFolders tempFolder;
    for (size_t i(0); i < lines.size(); ++i)
    {
        std::string l(extString::trim(lines[i]));
        std::string attributes(extString::between(l, "(", ")", false));
        IMAPFolder *f(new IMAPFolder);
        tempFolder.insert(f);
        extString::split(attributes, " ", f->attributes);
        l = extString::rightOf(l, "\"/\" ");
        l = extString::unQuote(l, "\"", "\\");
        extString::split(l, "/", f->name);
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

jw78::IMAPWrapper::IMAPFolder::IMAPFolder():parent(nullptr)
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
