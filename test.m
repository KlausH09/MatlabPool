clear
close all

global N pool

N = 50; % count of jobs in a single test
pool = MatlabPool(2);

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
    catch
        return % TODO check
    end
    error('no error thrown')
end