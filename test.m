clear
close all

global N pool

N = 50; % count of jobs in a single test
pool = MatlabPool(2);
pool.clear(); % cancel all jobs

count_eval.Small = 100;
count_eval.Normal = 30;
count_eval.Large = 5;
count_eval.Huge = 1;

i = 1;
while(true)
    name = sprintf('test_%d',i);
    fun = str2func(name);
    
    if(~exist(name,'file'))
        break
    end

    fprintf('run %9s...',name)
    fun();
    status = pool.statusJobs;
    if ~all(structfun(@(x)length(x),status) == 0)
        fprintf('there are jobs in the pool\n')
        error('there are jobs in the pool')
    end
    fprintf('passed\n')
    i = i+1;
end

%% Test 1: sqrt(i) with i = 0,1,...,N, double
function test_1() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,i);
    end
    for i = N:-1:1
        result = pool.wait(id(i));
        assert(abs(result{1}-sqrt(i)) < eps)
    end
end

%% Test 2: sqrt(i) with i = 0,1,...,N, float
function test_2() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,single(i));
    end
    for i = N:-1:1
        result = pool.wait(id(i));
        assert(abs(result{1}-sqrt(single(i))) < eps)
    end
end

%% Test 3: increase/decrease pool size
function test_3() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,i);
    end
    
    n = pool.size();
    pool.resize(pool.size() + 2);
    pool.resize(pool.size() - 2);
    assert(n == pool.size())
    
    for i = N:-1:1
        result = pool.wait(id(i));
        assert(abs(result{1}-sqrt(i)) < eps)
    end
end

%% Test 4: restart pool
function test_4() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,i); %#ok
    end
    
    n = pool.size();
    pool.delete();
    clear MatlabPoolMEX
    pool = MatlabPool(n);
end

%% Test 5: empty pool
function test_5() %#ok
    global pool
    try
        pool.resize(0);
    catch e
        if strcmp(e.identifier,'MatlabPoolMEX:EmptyPool')
            return
        end
    end
    error('no error thrown')
end

%% Test 6: wait for undefined job
function test_6() %#ok
    global pool
    try
        pool.wait(uint64(0));
    catch e
        if strcmp(e.identifier,'MatlabPoolMEX:JobNotExists')
            return
        end
    end
    error('no error thrown')
end

%% Test 7: wait x2 for same job
function test_7() %#ok
    global pool
    id = pool.submit('sqrt',1,47);
    pool.wait(id);
    try
        pool.wait(id);
    catch e
        if strcmp(e.identifier,'MatlabPoolMEX:JobNotExists')
            return
        end
    end
    error('no error thrown')
end

%% Test 8: eval
function test_8() %#ok
    global pool
    pool.eval('pwd');
end

%% Test 9: jobs and eval
function test_9() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,i);
    end
    pool.eval('pwd');
    for i = N:-1:1
        result = pool.wait(id(i));
        assert(abs(result{1}-sqrt(i)) < eps)
    end
end

%% Test 10: invalid eval
function test_10() %#ok
    global pool
    try
        pool.eval('pwwwd')
    catch e
        if strcmp(e.identifier,'MatlabPoolMEX:JobExecutionError')
            return
        end
    end
end

%% Test 11: invalid job
function test_11() %#ok
    global pool
    id = pool.submit('sqqqrt',1,47);
    try
        pool.wait(id);
    catch e
        if strcmp(e.identifier,'MatlabPoolMEX:JobExecutionError')
            return
        end
    end
    error('no error thrown')
end

%% Test 12: job with disp
function test_12() %#ok
    global pool
    id = pool.submit('disp',0,'Hello World');
    pool.wait(id);
end

%% Test 13: get job status
function test_13() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,i);
    end
    status = pool.statusJobs;
    assert(length(intersect(fieldnames(status),{'Status','WorkerID','JobID'})) == 3)
    assert(all(structfun(@(x)length(x),status) == N))
    for i = N:-1:1
        result = pool.wait(id(i));
        assert(abs(result{1}-sqrt(i)) < eps)
    end
end

%% Test 14: cancel jobs
function test_14() %#ok
    global N pool
    time = 0.001;
    for i = N:-1:1
        id(i) = pool.submit('pause',1,time);
    end
    for i = 1:N
        pool.cancel(id(i));
    end
end

%% Test 15: get worker status
function test_15() %#ok
    global N pool
    for i = N:-1:1
        id(i) = pool.submit('sqrt',1,i);
    end
    status = pool.statusWorker;
    assert(length(intersect(fieldnames(status),{'Ready'})) == 1)
    assert(all(structfun(@(x)uint64(length(x)),status) == pool.size()))
    for i = N:-1:1
        result = pool.wait(id(i));
        assert(abs(result{1}-sqrt(i)) < eps)
    end
end

%% Test 16: clear pool
function test_16() %#ok
    global N pool
    for i = N:-1:1
        pool.submit('sqrt',1,i);
    end
    pool.clear();
end