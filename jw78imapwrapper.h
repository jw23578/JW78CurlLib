#ifndef IMAPWRAPPER_H
#define IMAPWRAPPER_H

#include <string>
#include <set>
#include <vector>
#include <jw78emailwrapper.h>

namespace  jw78 {

class IMAPWrapper
{    
    const std::string host;
    const int port;
    const std::string user;
    const std::string password;
    std::string url;
public:
    class IMAPFolder
    {
    public:
        IMAPFolder *parent;
        int messages;
        int recent;
        int unseen;
        IMAPFolder();
        std::vector<std::string> name;
        std::set<std::string> attributes;
        std::set<IMAPFolder*> subFolder;
        std::vector<EMailWrapper*> emails;
        bool addFolder(IMAPFolder *newFolder);
        const std::string &getNamePart(int index);
        const std::string &getName();
        std::string getFullName();
        std::string getInfoString();
    };
    typedef std::set<IMAPFolder*> IMAPFolders;
    IMAPFolders allFolders;

    IMAPWrapper(const std::string &host,
                const int port,
                const std::string &user,
                const std::string &password);
    bool fetchFolder(std::string &errorMessage);
    bool updateFolder(IMAPFolder &folder);
    bool fetchEMail(const std::string &uid);
    bool fetchHeader(const std::string &uid,
                     std::string &header);
    bool fetchText(const std::string &uid,
                   std::string &text);
};

}


#endif // IMAPWRAPPER_H
