#ifndef IMAPWRAPPER_H
#define IMAPWRAPPER_H

#include <string>
#include <set>
#include <vector>

namespace  jw78 {

class IMAPWrapper
{    
    const std::string host;
    const int port;
    const std::string user;
    const std::string password;
public:
    class IMAPFolder
    {
    public:
        IMAPFolder *parent;
        IMAPFolder();
        std::vector<std::string> name;
        std::set<std::string> attributes;
        std::set<IMAPFolder*> subFolder;
        bool addFolder(IMAPFolder *newFolder);
        const std::string &getNamePart(int index);
        const std::string &getName();
        std::string getFullName();
    };
    typedef std::set<IMAPFolder*> IMAPFolders;
    IMAPFolders allFolders;

    IMAPWrapper(const std::string &host,
                const int port,
                const std::string &user,
                const std::string &password);
    bool fetchFolder(std::string &errorMessage);
};

}


#endif // IMAPWRAPPER_H
