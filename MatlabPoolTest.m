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
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_feval2(~)
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,single(i));
            end
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(single(i))) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_increas_decrease_poolSize(~)
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            n = MatlabPool.size();
            MatlabPool.resize(n + 2);
            assert(n + 2 == MatlabPool.size())
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(i)) < eps)
            end

            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            MatlabPool.resize(n);
            assert(n == MatlabPool.size())
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_clearPool(~)
            for i = MatlabPoolTest.N:-1:1
                MatlabPool.submit('sqrt',1,i);
            end
            MatlabPool.clear();
            MatlabPoolTest.check_is_empty()
        end

        function test_waitForUndefJob(~)
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
            MatlabPool.eval('pwd');
            MatlabPoolTest.check_is_empty()
        end

        function test_jobs_and_eval(~)
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            MatlabPool.eval('pwd');
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_invalid_eval(~)
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
            id = MatlabPool.submit('sqqqrt',1,47);
            try
                MatlabPool.wait(id);
                MatlabPoolTest.check_is_empty()
            catch e
                if strcmp(e.identifier,'MatlabPoolMEX:JobExecutionError')
                    return
                end
            end
            error('no error thrown')
        end

        function test_job_with_disp(~)
            id = MatlabPool.submit('disp',0,'Hello World');
            MatlabPool.wait(id);
            MatlabPoolTest.check_is_empty()
        end

        function test_jobStatus(~)
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            status = MatlabPool.statusJobs;
            assert(length(intersect(fieldnames(status),...
                   {'Status','WorkerID','JobID'})) == 3)
            assert(all(structfun(@(x)length(x),status) == MatlabPoolTest.N))
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_cancelJobs(~)
            time = 0.001;
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('pause',1,time);
            end
            for i = 1:MatlabPoolTest.N
                MatlabPool.cancel(id(i));
            end
            MatlabPoolTest.check_is_empty()
        end

        function test_workerStatus(~)
            for i = MatlabPoolTest.N:-1:1
                id(i) = MatlabPool.submit('sqrt',1,i);
            end
            status = MatlabPool.statusWorker;
            assert(length(intersect(fieldnames(status),{'Ready'})) == 1)
            assert(all(structfun(@(x)uint64(length(x)),status) == MatlabPool.size()))
            for i = MatlabPoolTest.N:-1:1
                result = MatlabPool.wait(id(i));
                assert(abs(result{1}-sqrt(i)) < eps)
            end
            MatlabPoolTest.check_is_empty()
        end
    end
    
end 