#ifndef SMTPWRAPPER_H
#define SMTPWRAPPER_H

#include <set>
#include <map>
#include <vector>
#include <thread>
#include "utils/threadsafequeue.h"

namespace jw78
{
class SMTPSender;

class SMTPWrapper
{
    friend SMTPSender;
private:
    static std::string defaultUser, defaultHost, defaultPassword;
    bool base64Mail;
    std::set<std::string> skip_lines;
    void internalSend();
    std::string fromName;
    std::string from_addr;
    std::string getEMLFrom(const std::string &prefix);
protected:
    ~SMTPWrapper() {}
public:
    static void setDefaultHostUserPassword(const std::string &host,
                                           const std::string &user,
                                           const std::string &password);
    void setFrom(std::string const &name,
                 std::string const &email);
    std::string host, user, password;
    std::vector<std::string> to_addr;
    std::vector<std::string> cc_addr;
    std::vector<std::string> bcc_addr;
    std::string reply_to;
    std::string content;
    std::string base64content;
    size_t content_position;
    SMTPWrapper();
    SMTPWrapper(std::string const &eml_file,
                std::string const &new_from_addr,
                std::string const &new_from_name);
    bool set_eml_file(std::string const &filename);
    void set_variable(std::string const &name,
                      std::string const &value);
    void setVariables(const std::map<std::string, std::string> &variables);
    void rawReplace(std::string const &needle,
                    std::string const &newValue);
    bool send();
    void createEmptyEMail(const std::string &subject,
                          const std::string &text,
                          const std::string &fromName,
                          const std::string &fromEMail);
};

class SMTPSender
{
private:
    bool stopped;
    std::thread theThread;
    ThreadSafeQueue<SMTPWrapper*> mailsToSend;
    SMTPSender();
    static SMTPSender theInstance;
public:
    static SMTPSender &gi();
    void theThreadFunction();
    void add(SMTPWrapper *email);
};
void SMTPSender_hook(SMTPSender *sender);

}




#endif // SMTPWRAPPER_H
