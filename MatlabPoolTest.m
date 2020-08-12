classdef MatlabPoolTest < matlab.unittest.TestCase

    properties(Constant)
        N = 50; % count jobs for each test
        nof_worker = 2;
    end
    
    methods
        function obj = MatlabPoolTest()
            MatlabPool.init(MatlabPoolTest.nof_worker);
            MatlabPool.clear();
        end
    end
    
    methods(Static)
        function check_is_empty()
            status = MatlabPool.statusJobs();
            assert(all(structfun(@(x)length(x),status) == 0),...
                   'there are jobs in the pool')
        end
    end
    
    methods (Test)
        function test_feval1(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(isempty(result.outputBuf))
                assert(isempty(result.errorBuf))
                assert(abs(result.result-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_feval2(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,single(i));
            end
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(isempty(result.outputBuf))
                assert(isempty(result.errorBuf))
                assert(abs(result.result-sqrt(single(i))) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_increas_decrease_poolSize(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            n = MatlabPool.size();
            MatlabPool.resize(n + 2);
            assert(n + 2 == MatlabPool.size())
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(isempty(result.outputBuf))
                assert(isempty(result.errorBuf))
                assert(abs(result.result-sqrt(i)) < eps)
            end

            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            MatlabPool.resize(n);
            assert(n == MatlabPool.size())
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(isempty(result.outputBuf))
                assert(isempty(result.errorBuf))
                assert(abs(result.result-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_clearPool(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                MatlabPool.submit('sqrt',1,i);
            end
            MatlabPool.clear();
            MatlabPoolTest.check_is_empty()
        end

        function test_waitForUndefJob(~)
            MatlabPool.clear();
            try
                MatlabPool.wait(uint64(0));
                MatlabPoolTest.check_is_empty()
            catch e
                if strcmp(e.identifier,'MatlabPoolMEX:JobNotExists')
                    return
                end
            end
            error('no error thrown')
        end

        function test_wait_2times_same_job(~)
            MatlabPool.clear();
            id = MatlabPool.submit('sqrt',1,47);
            MatlabPool.wait(id);
            try
                MatlabPool.wait(id);
                MatlabPoolTest.check_is_empty()
            catch e
                if strcmp(e.identifier,'MatlabPoolMEX:JobNotExists')
                    return
                end
            end
            error('no error thrown')
        end

        function test_eval(~)
            MatlabPool.clear();
            result = MatlabPool.eval('pwd');
            assert(~isempty(result.outputBuf))
            assert(isempty(result.errorBuf))
            MatlabPoolTest.check_is_empty()
        end

        function test_jobs_and_eval(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            result = MatlabPool.eval('pwd');
            assert(~isempty(result.outputBuf))
            assert(isempty(result.errorBuf))
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(isempty(result.outputBuf))
                assert(isempty(result.errorBuf))
                assert(abs(result.result-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_invalid_eval(~)
            MatlabPool.clear();
            try
                MatlabPool.eval('pwwwd')
            catch e
                if strcmp(e.identifier,'MatlabPoolMEX:JobExecutionError')
                    return
                end
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_invalid_job(~)
            MatlabPool.clear();
            id = MatlabPool.submit('sqqqrt',1,47);
            result = MatlabPool.wait(id);
            assert(isempty(result.outputBuf))
            assert(~isempty(result.errorBuf))
            MatlabPoolTest.check_is_empty()
        end

        function test_job_with_disp(~)
            MatlabPool.clear();
            id = MatlabPool.submit('disp',0,'Hello World');
            result = MatlabPool.wait(id);
            assert(~isempty(result.outputBuf))
            assert(isempty(result.errorBuf))
            MatlabPoolTest.check_is_empty()
        end

        function test_jobStatus(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            status = MatlabPool.statusJobs;
            assert(length(intersect(fieldnames(status),...
                   {'Status','WorkerID','JobID'})) == 3)
            assert(all(structfun(@(x)length(x),status) == MatlabPoolTest.N))
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result.result-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_cancelJobs(~)
            MatlabPool.clear();
            time = 0.001;
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('pause',1,time);
            end
            for i = 1:MatlabPoolTest.N
                MatlabPool.cancel(id(i));
            end
            MatlabPoolTest.check_is_empty()
        end
        
        function test_cancelEndlessJobs(~)
            MatlabPool.clear();
            time = 1e10;
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('pause',1,time);
            end
            for i = 1:MatlabPoolTest.N
                MatlabPool.cancel(id(i));
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_workerStatus(~)
            MatlabPool.clear();
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            status = MatlabPool.statusWorker;
            assert(length(intersect(fieldnames(status),{'Ready'})) == 1)
            assert(all(structfun(@(x)uint64(length(x)),status) == MatlabPool.size()))
            for i = MatlabPoolTest.N:-1:1
                MatlabPool.wait(id(i));
            end
            MatlabPoolTest.check_is_empty()
        end
    end
    
end 