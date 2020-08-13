clear
close all

addpath([pwd '/build/lib'])

width = 4096;
height = 4096;
blocksize = [1024,512,256,128,64];
countJob = ceil(width./blocksize).*ceil(height./blocksize);

workersize = 0:3;

for j = length(workersize):-1:1
    for i = length(blocksize):-1:1
        time(i,j) = timejulia(width, height,...
                       blocksize(i), workersize(j));
    end
end

ax = gca;
plot(ax,time,'-*','linewidth',2)
ax.XTick = 1:length(countJob);
ax.XTickLabel = arrayfun(@num2str,countJob,'uni',0);
ylabel('time [sec]')
xlabel('count jobs')
legend(['no pool', ...
    arrayfun(@(x)sprintf('%d worker',x),workersize(2:end),'uni',0)])
title('compute 4096x4096 Juila set picture')


function time = timejulia(width, height, blocksize, workersize)

% julia settings 
a = 1.7;
c = .327+.22i;
max_iter = uint16(80);
max_val = 2^16;

% define jobs
USlice = fractal.UniformSlices([width,height],...
    [ceil(height/blocksize),...
     ceil(width/blocksize)]);
 
Z = linspace(-1.8,1.8,width)...
   +1i*linspace(-1.9,1.9,height)'; 
Z = USlice.getSlicesStruct(Z);


if workersize == 0
    tic
    for i = length(Z):-1:1
        result{i} = fractal.julia(Z(i),a,c,max_iter,max_val);
    end
    time = toc;
else
    
    MatlabPool.resize(workersize);
    tic
    for i = length(Z):-1:1
        id(i) = MatlabPool.submit('fractal.julia',1,...
                            Z(i),a,c,max_iter,max_val);
    end
    for i = length(Z):-1:1
        tmp = MatlabPool.wait(id(i));
        result{i} = tmp.result;
    end
    time = toc;
    
end

USlice.mergeSlices(result);
end