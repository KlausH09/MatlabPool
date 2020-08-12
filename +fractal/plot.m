function plot(ax,n,max_iter)
arguments
    ax(1,1)
    n(:,:)
    max_iter(1,1)
end

if nargin < 2
    max_iter = max(n,[],'all'); 
end

fun = fractal.get_colormap();
rgbimg = fun(double(n)/double(max_iter));

image(ax,rgbimg)
ax.XTick = [];
ax.YTick = [];

        
end