#include "jw78smtpwrapper.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include "jw78curlwrapper.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <ctime>
#include "jw78core_debug.h"
#include "jw78core_helper.h"
#include <base64.h>


std::string jw78::SMTPWrapper::defaultUser;
std::string jw78::SMTPWrapper::defaultHost;
std::string jw78::SMTPWrapper::defaultPassword;

jw78::SMTPWrapper::SMTPWrapper(std::string const &eml_file,
                               std::string const &new_from_addr,
                               std::string const &new_from_name):
    base64Mail(false)
{
    skip_lines.insert("FCC:");
    skip_lines.insert("X-Identity-Key:");
    skip_lines.insert("X-Account-Key:");
    skip_lines.insert("User-Agent:");
    skip_lines.insert("FCC:");
    skip_lines.insert("Cc:");
    skip_lines.insert("BCC:");
    skip_lines.insert("To:");
    setFrom(new_from_name,
            new_from_addr);
    set_eml_file(eml_file);
}


bool jw78::SMTPWrapper::send()
{
    SMTPSender::gi().add(this);
    return true;
}

jw78::SMTPWrapper::SMTPWrapper()
{
}

void jw78::SMTPWrapper::createEmptyEMail(std::string const &subject,
                                         std::string const &text,
                                         std::string const &fromName,
                                         std::string const &fromEMail)
{
    setFrom(fromName,
            fromEMail);
    content = "";

    content += getEMLFrom("From: ");

    content += "Subject: " + subject + "\r\n";

    content += getEMLFrom("Reply-To: ");

    std::string work = "Message-ID: ";
    work += boost::uuids::to_string(boost::uuids::random_generator()());
    work += "\r\n";
    content += work;

    work = "Date: ";
    time_t current(0);
    std::time(&current);
    char rfc_2822[40];
    std::strftime(rfc_2822,
                  sizeof(rfc_2822),
                  "%a, %d %b %Y %T %z",
                  std::localtime(&current));
    work += rfc_2822;
    work += "\r\n";
    content += work;

    content += "MIME-Version: 1.0\r\n";
    content += "Content-Type: text/html; charset=utf-8\r\n";
    content += "Content-Language: en-US\r\n";
    content += "Content-Transfer-Encoding: base64\r\n";
    content += "\r\n";

    std::string html;
    html += "<html>\r\n";
    html += "<head>\r\n";
    html += "\r\n";
    html += "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\r\n";
    html += "</head>\r\n";
    html += "<body text=\"#000000\" bgcolor=\"#FFFFFF\">\r\n";
    html += text;
    html += "</body>\r\n";
    html += "</html>\r\n";

    base64content = html;
}

void jw78::SMTPWrapper::internalSend()
{
    if (base64Mail)
    {
        base64content = base64_encode((unsigned char*)base64content.c_str(), base64content.size());
        size_t pos(0);
        while (pos < base64content.size())
        {
            content += base64content.substr(pos, 72);
            content += "\r\n";
            pos += 72;
        }
    }
    std::string receiver;
    for (size_t i(0); i < to_addr.size(); ++i)
    {
        receiver += "To: ";
        receiver += to_addr[i];
        receiver += "\r\n";
    }
    for (size_t i(0); i < cc_addr.size(); ++i)
    {
        receiver += "Cc: ";
        receiver += cc_addr[i];
        receiver += "\r\n";
    }
    content = receiver + content;
    jw78core::debug::gi()->d(__FILE__, __LINE__, std::string("from: ") + from_addr);
    jw78core::debug::gi()->d(__FILE__, __LINE__, std::string("content: ") + content.substr(0, 200));
    if (host == "")
    {
        host = defaultHost;
        password = defaultPassword;
        user = defaultUser;
    }
    CurlWrapper curlWrapper;
    std::string logResult;
    curlWrapper.smtpSendMail(host,
                             user,
                             password,
                             from_addr,
                             to_addr,
                             cc_addr,
                             bcc_addr,
                             content,
                             logResult);
    jw78core::debug::gi()->d(__FILE__, __LINE__, std::string("smtp logResult: ") + logResult);
}

std::string jw78::SMTPWrapper::getEMLFrom(const std::string &prefix)
{
    std::string work(prefix);
    if (fromName.size())
    {
        work += fromName + " ";
    }
    work += from_addr;
    work += "\r\n";
    return work;
}

void jw78::SMTPWrapper::setDefaultHostUserPassword(const std::string &host,
                                                   const std::string &user,
                                                   const std::string &password)
{
    defaultHost = host;
    defaultUser = user;
    defaultPassword = password;
}

void jw78::SMTPWrapper::setFrom(const std::string &name, const std::string &email)
{
    if (email.size() == 0)
    {
        fromName = "";
        from_addr = "";
        return;
    }
    fromName = name;
    from_addr = std::string("<") + email + ">";
}

bool jw78::SMTPWrapper::set_eml_file(std::string const &filename)
{
    std::ifstream eml(filename);
    if (!eml)
    {
        return false;
    }
    content = "";
    std::string newLine("\r\n");
    while (eml)
    {
        char buf[1000];
        eml.getline(buf, 1000);
        std::string work(buf);
        boost::algorithm::erase_all(work, "\r");
        boost::algorithm::erase_all(work, "\n");
        std::set<std::string>::iterator it(skip_lines.begin());
        bool skip(false);
        while (it != skip_lines.end())
        {
            if (work.find(*it) == 0)
            {
                skip = true;
            }
            ++it;
        }
        if (skip)
        {
            continue;
        }
        if (work.find("From:") == 0)
        {
            content += from_addr.size() ? getEMLFrom("From: ") : work + newLine;
            continue;
        }
        if (work.find("Reply-To:") == 0)
        {
            content += from_addr.size() ? getEMLFrom("Reply-To: ") : work + newLine;
            continue;
        }
        if (work.find("Message-ID") == 0)
        {
            work = "Message-ID: ";
            work += boost::uuids::to_string(boost::uuids::random_generator()());
            work += newLine;
            content += work;
            continue;
        }
        if (work.find("Date") == 0)
        {
            work = "Date: ";
            time_t current(0);
            std::time(&current);
            char rfc_2822[40];
            std::strftime(rfc_2822,
                          sizeof(rfc_2822),
                          "%a, %d %b %Y %T %z",
                          std::localtime(&current));
            work += rfc_2822;
            work += newLine;
            content += work;
            continue;
        }
        if (work == "Content-Transfer-Encoding: base64")
        {
            base64Mail = true;
        }
        if (work == "")
        {
            content += work + newLine;
            while (eml)
            {
                eml.getline(buf, 1000);
                work = buf;
                boost::algorithm::erase_all(work, "\r");
                boost::algorithm::erase_all(work, "\n");
                if (base64Mail)
                {
                    base64content += work;
                }
                else
                {
                    content += work + newLine;
                }
            }
            if (base64Mail)
            {
                base64content = base64_decode(base64content);
            }
            return true;
        }
        //        if (work.find("Content") == 0)
        //        {
        //            content += work + std::string("\r\n");
        //            while (eml)
        //            {
        //                eml.getline(buf, 1000);
        //                work = buf;
        //                content += work + std::string("\r\n");
        //            }
        //            return true;
        //        }
        content += work + newLine;
    }
    return false;
}

void jw78::SMTPWrapper::set_variable(const std::string &name,
                                  const std::string &value)
{
    if (base64Mail)
    {
        boost::algorithm::replace_all(base64content, std::string("xxxxx") + name + "xxxxxx", value);
        boost::algorithm::replace_all(base64content, std::string("((") + name + "))", value);
        boost::algorithm::replace_all(base64content, std::string("{{") + name + "}}", value);
        boost::algorithm::replace_all(base64content, std::string("%7B%7B") + name + "%7D%7D", value);
        boost::algorithm::replace_all(base64content, std::string("&amp;") + name + "&amp;", value);
        boost::algorithm::replace_all(base64content, std::string("&") + name + "&", value);
    }
    else
    {
        boost::algorithm::replace_all(content, std::string("xxxxx") + name + "xxxxxx", value);
        boost::algorithm::replace_all(content, std::string("((") + name + "))", value);
        boost::algorithm::replace_all(content, std::string("{{") + name + "}}", value);
        boost::algorithm::replace_all(content, std::string("%7B%7B") + name + "%7D%7D", value);
        boost::algorithm::replace_all(content, std::string("&amp;") + name + "&amp;", value);
        boost::algorithm::replace_all(content, std::string("&") + name + "&", value);
    }
}

void jw78::SMTPWrapper::setVariables(const std::map<std::string, std::string> &variables)
{
    std::map<std::string, std::string>::const_iterator it(variables.begin());
    while (it != variables.end())
    {
        set_variable(it->first, it->second);
        ++it;
    }
}

void jw78::SMTPWrapper::rawReplace(const std::string &needle,
                                const std::string &newValue)
{
    if (base64Mail)
    {
        boost::algorithm::replace_all(base64content, needle, newValue);
    }
    else
    {
        boost::algorithm::replace_all(content, needle, newValue);
    }
}

void jw78::SMTPSender_hook(jw78::SMTPSender *sender)
{
    sender->theThreadFunction();
}

jw78::SMTPSender jw78::SMTPSender::theInstance;

jw78::SMTPSender::SMTPSender():stopped(false),
    theThread(SMTPSender_hook, this)
{
}

jw78::SMTPSender &jw78::SMTPSender::gi()
{
    return theInstance;
}

void jw78::SMTPSender::theThreadFunction()
{
    while (!stopped)
    {
        SMTPWrapper *m(mailsToSend.dequeue());
        m->internalSend();
        delete m;
    }
}

void jw78::SMTPSender::add(SMTPWrapper *email)
{
    mailsToSend.enqueue(email);
}
