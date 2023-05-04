f19200 = fopen('./19200_baud_samples', 'rb');
values_19200 = fread(f19200, Inf, 'float');
f4800 = fopen('./4800_baud_samples', 'rb');
values_4800 = fread(f4800, Inf, 'float');
figure (1);
stem(values_19200)
title("19200 Baud")
figure (2);
stem(values_4800)
title("4800 Baud")