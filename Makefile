# Compilador e flags
CC		= gcc
CFLAGS	= -Wall -O3 -fPIC `cups-config --cflags`
LDFLAGS	= 
LDLIBS	= `cups-config --libs` -lcupsimage -lcups

# Fontes e objetos
SRCS = rastertozj.c
OBJS = $(SRCS:.c=.o)

# Alvo padrão
all: rastertozj

# Linkagem final
rastertozj: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compilação dos objetos
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	$(RM) $(OBJS) rastertozj

# Instalação
install:
	./install.sh
