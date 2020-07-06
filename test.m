clear
close all

n = 2;
pool = MatlabPool(2);

for i = 100:-1:1
    id(i) = pool.submit('sqrt',1,i);
end

for i = 100:-1:1
    result(i) = pool.wait(id(i));
end