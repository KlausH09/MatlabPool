#ifndef MATLABPOOL_MEXCOMMANDS_HPP
#define MATLABPOOL_MEXCOMMANDS_HPP

#include "MatlabPoolMex.hpp"

namespace MatlabPool::MexCommands
{
    using CmdID = uint8_t;

    namespace
    {
        // define function pointer
        typedef void (MexFunction::*CmdFun)(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);

        typedef struct Cmd
        {
            const char *const name;
            const CmdFun fun;
        } Cmd;

        // define commands with name
        constexpr Cmd commands[] =
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

        constexpr CmdID nof_commands = CmdID(sizeof(commands) / sizeof(Cmd));
    } // namespace

    // get the name of a command
    inline const char *get_name(CmdID i) noexcept
    {
        static constexpr const char undef_cmd[] = "undefined_command";
        if (i >= nof_commands)
            return undef_cmd;

        return commands[i].name;
    }

    // get the function pointer of a command
    inline CmdFun get_fun(CmdID i)
    {
        if (i >= nof_commands)
            throw MexFunction::UndefCmd();

        return commands[i].fun;
        // call the function with:  (this->*Commands::get_fun(i))(...);
    }

} // namespace Commands

#endif