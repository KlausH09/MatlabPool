function fun = get_colormap()

persistent fun_tmp
if ~isempty(fun_tmp)
    fun = fun_tmp;
    return
end

color =  [   %66  30  15 ; % brown 3
             25   7  26 ; % dark violett
              9   1  47 ; % darkest blue
              4   4  73 ; % blue 5
              0   7 100 ; % blue 4
             12  44 138 ; % blue 3
             24  82 177 ; % blue 2
             57 125 209 ; % blue 1
            134 181 229 ; % blue 0
            211 236 248 ; % lightest blue
            241 233 191 ; % lightest yellow
            248 201  95 ; % light yellow
            255 170   0 ; % dirty yellow
            204 128   0 ; % brown 0
            153  87   0 ; % brown 1
            106  52   3 ; % brown 2
             66  30  15 ; % brown 3
              0   0   0 ;
         ];  
    
pos = linspace(0,1,size(color,1));
     
R = griddedInterpolant(pos,color(:,1)); 
G = griddedInterpolant(pos,color(:,2)); 
B = griddedInterpolant(pos,color(:,3));     
 
fun_tmp = @(x)cat(3,uint8(R(x)),uint8(G(x)),uint8(B(x))); 
fun = fun_tmp;

end