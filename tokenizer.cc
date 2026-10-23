
#include "tokenizer.h"
#include <sstream>
#include <iostream>

std::vector<Token> tokenize(const std::string &input)
{
    std::vector<Token> tokens;
    std::string word;
    bool inQuotes=false;
    char quoteChar='\0';
    for (int i=0;i<input.length(); i++)
    {
        char c=input[i];
        if ((c == '"' ||c == '\'')&&!inQuotes)
        {
            inQuotes=true;
            quoteChar=c;
            continue;
        }
        else if (inQuotes&& c== quoteChar)
        {
            inQuotes=false;
            continue;
        }
        else if ((c== ' ' || c == '\t')&&!inQuotes)
        {
            if (word.empty())
                continue;

            Token token;
            if (word == "|")
                token.type = TOKEN_PIPE;
            else if (word == ">")
                token.type = TOKEN_REDIRECT;
            else if (word == ">>")
                token.type = TOKEN_APPEND;
            else if (word == "<")
                token.type = TOKEN_INPUT;
            else if (word == "&")
                token.type = TOKEN_BACKGROUND;
            else if (word == "2>")
                token.type = TOKEN_ERROR;
            else if (word == ">>&")
                token.type = TOKEN_REDIRECT_AND_ERROR;
            else
            {
                if (!tokens.empty())
                {
                    Token &prev=tokens.back();

                    if (prev.type ==TOKEN_PIPE)
                    {
                        token.type=TOKEN_COMMAND;
                    }
                    else if (prev.type == TOKEN_REDIRECT ||
                             prev.type == TOKEN_APPEND ||
                             prev.type == TOKEN_INPUT ||
                             prev.type == TOKEN_ERROR ||
                             prev.type == TOKEN_REDIRECT_AND_ERROR ||
                             prev.type == TOKEN_COMMAND ||
                             prev.type == TOKEN_ARGUMENT)
                    {
                        token.type = TOKEN_ARGUMENT;
                    }
                    else{
                        token.type = TOKEN_COMMAND;
                    }
                }
                else{
                    token.type = TOKEN_COMMAND;
                }
            }
            token.value = word;
            tokens.push_back(token);
            word.clear();
        }
        else
        {
            word+=c;
        }
    }
    if (inQuotes)
    {
        std::cerr<<"Error: unmatched quotation mark" << std::endl;
        tokens.push_back({TOKEN_EOF, ""});
        return tokens;
    }
    if (!word.empty())
    {
        Token token;
        if (word == "|")
            token.type = TOKEN_PIPE;
        else if (word == ">")
            token.type = TOKEN_REDIRECT;
        else if (word == ">>")
            token.type = TOKEN_APPEND;
        else if (word == "<")
            token.type = TOKEN_INPUT;
        else if (word == "&")
            token.type = TOKEN_BACKGROUND;
        else if (word == "2>")
            token.type = TOKEN_ERROR;
        else if (word == ">>&")
            token.type = TOKEN_REDIRECT_AND_ERROR;
        else if (!tokens.empty() && tokens.back().type == TOKEN_PIPE)
            token.type = TOKEN_COMMAND;
        else if (!tokens.empty())
            token.type = TOKEN_ARGUMENT;
        else
            token.type = TOKEN_COMMAND;
        token.value = word;
        tokens.push_back(token);
    }

    tokens.push_back({TOKEN_EOF, ""});
    return tokens;
}
