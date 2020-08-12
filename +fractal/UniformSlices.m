classdef UniformSlices
    
properties(Access=private)
   
   dim(1,:)uint64 
   slices(1,:)uint64
   
   slices_dim(1,:)cell
end
    
methods(Access = public)

    function obj = UniformSlices(dim,slices)
        arguments
            dim(1,:) {mustBeInteger,mustBeNonempty}
            slices(1,:) {mustBeInteger,mustBeNonempty,mustBeGreaterThan(slices,0)}
        end

        assert(numel(dim) == numel(slices),'Assertion: ''numel(dim) == numel(slices)''')
        assert(all(slices <= dim),'Assertion: ''all(slices <= dim)''');
        
        obj.dim = dim;
        obj.slices = slices;
        
        tmp_dim = uint64(floor(double(dim)./double(slices)));
        reminder = obj.dim - tmp_dim .* obj.slices;
        
        for i = numel(dim):-1:1
            tmp = repmat(tmp_dim(i),1,slices(i));
            tmp(1:reminder(i)) = tmp(1:reminder(i)) + 1;
            obj.slices_dim{i} =  tmp;
        end
    end
    
    function s = getSlices(obj,A)
        assert(isequal(size(A),obj.dim),'Assertion: ''isequal(size(A),obj.dim)''')
        s = mat2cell(A,obj.slices_dim{:});
        s = s(:);
    end
    
    function s = getSlicesStruct(obj,A)
        A = obj.getSlices(A);
        s = cellfun(@(x)struct(...
                       'p0',x(1),...
                       'p1',x(end),...
                       'dim',size(x)),A);
    end
    
    function A = mergeSlices(obj,s)
        assert(numel(s)==prod(obj.slices),'Assertion: ''numel(s)==prod(obj.slices)''')
        A = cell2mat(reshape(s(:),obj.slices));
    end
    
    function n = count_slices(obj)
        n = prod(obj.slices);
    end
    
end
    
end
    