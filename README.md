# Epson TM-T20II (Filtro CUPS Modernizado)

Este projeto fornece um filtro CUPS em C para a impressora térmica **Epson TM-T20II**, compatível com sistemas ARM como Raspberry Pi, Orange Pi e servidores Linux compactos.

O driver oficial da Epson para Linux é um binário x86 e não funciona em arquiteturas ARM. Este filtro substitui essa limitação com uma solução leve, compilável e compatível com versões modernas do CUPS — incluindo o CUPS 3.x, que **não suporta arquivos `.ppd`**.

---

## 🚀 Principais recursos

- ✅ Compatível com CUPS 2.2+ e CUPS 3.x
- ✅ Sem dependência de arquivos `.ppd`
- ✅ Comandos ESC/POS para corte de papel e gaveta de dinheiro
- ✅ Otimização de impressão de linhas em branco
- ✅ Script de instalação automatizado (`install.sh`)
- ✅ Ideal para sistemas embarcados e PDVs

---

## 🔧 Requisitos para compilação

Instale os pacotes necessários:

```bash
sudo apt-get update
sudo apt-get install -y libcups2-dev libcupsimage2-dev g++ cups cups-client
```

---

## ⚙️ Instalação automatizada

Use o script `install.sh` para compilar e instalar o filtro:

```bash
chmod +x install.sh
./install.sh
```

O script irá:

- Verificar dependências
- Compilar o filtro `rastertozj`
- Detectar o diretório correto do CUPS
- Instalar o binário com permissões adequadas
- Orientar sobre a configuração da impressora

---

## 🖨️ Configuração da impressora no CUPS

1. Acesse o painel do CUPS:
   ```
   http://localhost:631
   ```

2. Vá em **Administration > Add Printer**

3. Selecione sua impressora Epson TM-T20II (USB ou rede)

4. Escolha um driver genérico:
   - **Raw**
   - Ou **Generic ESC/POS**

5. Nome sugerido:
   ```
   Epson_TM_T20II
   ```

6. Finalize e imprima uma página de teste

---

## 🧪 Alternativa via terminal

Você também pode registrar a impressora diretamente:

```bash
lpadmin -p Epson_TM_T20II -E -v usb://EPSON/TM-T20II -m raw
```

---

## 📁 Estrutura do projeto

```
├── rastertozj.c       # Código-fonte do filtro
├── install.sh         # Script de instalação automatizada
├── Makefile           # Regras de compilação
└── README.md          # Documentação do projeto
```

---

## ✅ Compatibilidade

- Raspberry Pi OS (32 e 64 bits)
- Ubuntu Server ARM64
- Debian ARM
- CUPS 2.2, 2.3, 2.4 e 3.x

---

## 📦 Licença

Este projeto é fornecido como está, sem garantias. Uso e modificação são livres conforme os termos da licença original.

---

Se quiser incluir suporte para outras impressoras térmicas ou expandir o script para configurar o CUPS automaticamente, posso te ajudar com isso também. Só chamar!
