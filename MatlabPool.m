classdef MatlabPool < handle
    
    properties(Constant,Access = private)
        cmd_resize       = uint8(0)
        cmd_submit       = uint8(1)
        cmd_wait         = uint8(2)
        cmd_statusJobs   = uint8(3)
        cmd_statusWorker = uint8(4)
        cmd_eval         = uint8(5)
        cmd_cancel       = uint8(6)
        cmd_size         = uint8(7)
        cmd_clear        = uint8(8)
        
        options = {'-nojvm', '-nosplash'}
    end
        
    methods(Static)
        
        function init(val)
            addpath([fileparts(mfilename('fullpath')) '/lib'])
            MatlabPool.resize(val);
        end
        
        function shutdown()
            clear('MatlabPoolMEX')
        end
        
        function jobid = submit(fun,nof_args,varargin)
            jobid = MatlabPoolMEX(MatlabPool.cmd_submit,fun,uint64(nof_args),varargin{:});
        end
        
        function result = wait(jobid)
            result = MatlabPoolMEX(MatlabPool.cmd_wait,uint64(jobid));
        end
        
        function status = statusJobs()
            status = MatlabPoolMEX(MatlabPool.cmd_statusJobs);
        end
        
        function status = statusWorker()
            status = MatlabPoolMEX(MatlabPool.cmd_statusWorker);
        end
        
        function eval(fun)
            MatlabPoolMEX(MatlabPool.cmd_eval,fun);
        end
        
        function cancel(jobid)
            MatlabPoolMEX(MatlabPool.cmd_cancel,uint64(jobid));
        end
        
        function val = size()
            val = MatlabPoolMEX(MatlabPool.cmd_size);
        end

        function clear()
            MatlabPoolMEX(MatlabPool.cmd_clear);
        end
        
        function resize(val)
            val = uint32(val);
            if val == uint32(0)
                MatlabPool.shutdown();
            else
                MatlabPoolMEX(MatlabPool.cmd_resize,val,MatlabPool.options{:})
            end    
        MatlabPoolMEX(MatlabPool.cmd_resize,val,MatlabPool.options{:})
        end
        
    end
end