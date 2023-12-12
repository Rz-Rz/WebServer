#ifndef MULTIPARTFORMDATAPARSER_HPP
#define MULTIPARTFORMDATAPARSER_HPP

#include <string>
#include <map>
#include <vector>



class MultipartFormDataParser {
  public:
    MultipartFormDataParser(const std::string& body, const std::string& boundary)
        : body(body), boundary(boundary) {}

    void parse();

    std::map<std::string, std::string> getFormFields() const;
    std::map<std::string, std::string> getFileFields() const;

    class MultipartFormDataParserException : public std::exception {
      private:
        std::string message;

      public:
        MultipartFormDataParserException(const std::string& msg);
        virtual const char* what() const throw();
        virtual ~MultipartFormDataParserException() throw();
    };
private:
    std::string body;
    std::string boundary;
    std::map<std::string, std::string> formFields;
    std::map<std::string, std::string> fileFields;

    std::vector<std::string> splitBodyByBoundary();
    void parsePart(const std::string& part);
    std::map<std::string, std::string> parseHeaders(const std::string& part) const;
    std::string extractContent(const std::string& part) const;
    void trim(std::string& str) const;
    void parseDisposition(const std::string& disposition, std::string& name, std::string& filename);

};


#endif
