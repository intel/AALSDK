#pragma once 
class afu_register
{
    public:
        afu_register(const std::string &id,
                     const std::string &offset,
                     const std::string &type,
                     const std::string &width,
                     const std::string &comment) :
            id_(id),
            type_(type),
            comment_(comment)
        {
            offset_ = std::strtol(offset.c_str(), nullptr, 16);
            width_ = std::strtol(width.c_str(), nullptr, 10);
        }
        
        afu_register(const std::string &id,
                     const unsigned int offset,
                     const std::string &type,
                     const unsigned int width,
                     const std::string &comment) :
            id_(id),
            offset_(offset),
            type_(type),
            width_(width),
            comment_(comment){}

        const std::string & id()
        {
            return id_;
        }

        const std::string & id() const
        {
            return id_;
        }

        const unsigned int & offset()
        {
            return offset_;
        }

        const std::string & type()
        {
            return type_;
        }

        const unsigned int & width()
        {
            return width_;
        }

        const std::string & comment()
        {
            return comment_;
        }

    private:
        std::string id_;
        unsigned int offset_;
        std::string type_;
        unsigned int width_;
        std::string comment_;
};
