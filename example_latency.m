clear
close all

MatlabPool.resize(1);

n = 1000;

tic
for i = 1:n
    id = MatlabPool.submit('pause',0,0);
    MatlabPool.wait(id);
end
t = toc/n;
fprintf('latency time 1: %.2e sec\n',t)
