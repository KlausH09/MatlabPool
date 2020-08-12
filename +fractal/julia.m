function n = julia(C,a,c,n_max,val_max)
    arguments
        C(1,1)struct
        a(1,1)double
        c(1,1)
        n_max(1,1)uint16
        val_max(1,1)double
    end

    C = linspace(real(C.p0),real(C.p1),C.dim(2)) ...
       +1i*linspace(imag(C.p0),imag(C.p1),C.dim(1))';
    
    id = true(size(C));
    n  = zeros(size(C),'uint8');    

    t = uint64(0);
    while any(id,'all') && t<n_max
        C(id) = C(id).^a + c;
        id(id) = abs(C(id)) <= val_max;
        n(id) = n(id)  + 1;
        t = t+1;
    end

end
