#ifndef MATLABPOOL_MEXCOMMANDS_HPP
#define MATLABPOOL_MEXCOMMANDS_HPP

#include "MatlabPoolMEX.hpp"

namespace MatlabPool
{

    class MexCommands
    {
    public:
        using CmdID = uint8_t;

    private:
        // define function pointer
        typedef void (MexFunction::*CmdFun)(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);

        typedef struct Cmd
        {
            const char *const name;
            const CmdFun fun;
        } Cmd;

        // define commands with name
        inline static constexpr Cmd commands[] =
            {
                /*  0 */ {"resize", &MexFunction::resize},
                /*  1 */ {"submit", &MexFunction::submit},
                /*  2 */ {"wait", &MexFunction::wait},
                /*  3 */ {"statusJobs", &MexFunction::statusJobs},
                /*  4 */ {"statusWorker", &MexFunction::statusWorker},
                /*  5 */ {"eval", &MexFunction::eval},
                /*  6 */ {"cancel", &MexFunction::cancel},
                /*  7 */ {"size", &MexFunction::size},
                /*  8 */ {"clear", &MexFunction::clear},
        };

        inline static constexpr CmdID nof_commands = CmdID(sizeof(commands) / sizeof(Cmd));

    public:
        class MexCommandsException : public Exception
        {
        };
        class UndefCmd : public MexCommandsException
        {
        public:
            inline const char *what() const noexcept override
            {
                return "Undefined command";
            }
            inline const char *identifier() const noexcept override
            {
                return "UndefCMD";
            }
        };

    public:
        // get the name of a command
        inline static const char *get_name(CmdID i) noexcept
        {
            if (i >= nof_commands)
                return "undefined_command";

            return commands[i].name;
        }

        // get the function pointer of a command
        inline static CmdFun get_fun(CmdID i)
        {
            if (i >= nof_commands)
                throw UndefCmd();

            return commands[i].fun;
            // call the function with:  (this->*Commands::get_fun(i))(...);
        }
    };
    namespace
    {

    } // namespace

} // namespace MatlabPool

#endif