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
        
        options = {'-nojvm', '-nosplash'}
    end
    
    properties(SetAccess = private)
        n(1,1)uint32
    end
    
    methods
        function obj = MatlabPool(n)
            resize(obj,n)
        end
        
        function delete(~)
            % clear MexMatlabPool
        end
        
        function jobid = submit(~,fun,nof_args,varargin)
            jobid = MexMatlabPool(MatlabPool.cmd_submit,fun,uint64(nof_args),varargin{:});
        end
        
        function result = wait(~,jobid)
            result = MexMatlabPool(MatlabPool.cmd_wait,uint64(jobid));
        end
        
        function status = statusJobs(~)
            status = MexMatlabPool(MatlabPool.cmd_statusJobs);
        end
        
        function status = statusWorker(~)
            status = MexMatlabPool(MatlabPool.cmd_statusWorker);
        end
        
        function eval(~,fun)
            MexMatlabPool(MatlabPool.cmd_eval,fun);
        end
        
        function cancel(~,jobid)
            MexMatlabPool(MatlabPool.cmd_cancel,uint64(jobid));
        end
        
        function val = size(~)
            val = MexMatlabPool(MatlabPool.cmd_size);
        end
        
        function resize(obj,val)
            obj.n = val;
            MexMatlabPool(MatlabPool.cmd_resize,obj.n,MatlabPool.options{:})
        end
        
    end
end