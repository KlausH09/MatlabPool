clear
close all

addpath([pwd '/build/lib'])

MatlabPool.resize(2);

%% dimensions
width = 3000;
heigth = 3000;
blocksize = 128;

%% julia settings 
a = 1.7;
c = .327+.22i;
max_iter = uint16(80);
max_val = 2^16;

%% define jobs
USlice = fractal.UniformSlices([width,heigth],...
    [ceil(heigth/blocksize),...
     ceil(width/blocksize)]);
 
Z = linspace(-1.8,1.8,width)...
   +1i*linspace(-1.9,1.9,heigth)'; 
Z = USlice.getSlicesStruct(Z);

%% without MatlabPool
tic
for i = length(Z):-1:1
    n_single{i} = fractal.julia(Z(i),a,c,max_iter,max_val);
end
t_single = toc;
n_single = USlice.mergeSlices(n_single);

%% with MatlabPool
tic
for i = length(Z):-1:1
    id(i) = MatlabPool.submit('fractal.julia',1,...
                        Z(i),a,c,max_iter,max_val);
end
for i = length(Z):-1:1
    tmp = MatlabPool.wait(id(i));
    n_parallel{i} = tmp.result;
end
t_parallel = toc;
n_parallel = USlice.mergeSlices(n_parallel);

%% disp results
ax = subplot(1,2,1);
fractal.plot(ax,n_single,max_iter)
title(sprintf('without MatlabPool: %.2f sec',t_single))

ax = subplot(1,2,2);
fractal.plot(ax,n_parallel,max_iter)
title(sprintf('with MatlabPool: %.2f sec',t_parallel))
