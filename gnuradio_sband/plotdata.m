f = fopen('decodeddata_quarterrate.bin', 'rb');
values = fread(f, Inf, 'char');
figure(1);
stem(values(147500:160000));#starts in sync mode, then goes to 01010101, then.... packet data?
figure(2);
marker = [0 0 0 1 1 0 1 0 1 1 0 0 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 1 1 1 0 1]; #marker found at sample ~133000
quinten = [0 1 0 1 0 0 0 1 0 1 1 1 0 1 0 1 0 1 1 0 0 1 0 1 0 1 1 0 1 1 1 0 0 1 1 1 0 1 0 0]
stem(marker);
figure(3)
stem(quinten);


#sync data isn't what it should be... most likely decoding incorrect.