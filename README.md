# Epson TM-T20II (CUPS Filter Modernizado)

Filtro CUPS para impressora térmica Epson TM-T20II, compatível com sistemas ARM como Raspberry Pi, Orange Pi e servidores Linux compactos.

Este projeto substitui o driver oficial da Epson (que é x86 e incompatível com ARM) por um filtro em C que usa a API moderna do CUPS (`cupsCopyDestInfo`), compatível com CUPS 2.2+ e preparado para CUPS 3.x (sem suporte a PPDs).

---

## 🔧 Requisitos para Compilação

Instale os pacotes necessários:

```bash
sudo apt-get install libcups2-dev libcupsimage2-dev g++ cups cups-client
```

Compile o filtro:

```bash
make
```

---

## 📁 Instalação Após a Compilação

Instale o binário no diretório de filtros do CUPS:

```bash
sudo cp rastertozj /usr/lib/cups/filter/
sudo chmod +x /usr/lib/cups/filter/rastertozj
```

Verifique o caminho correto com:

```bash
cups-config --serverbin
```

---

## 🖨️ Configuração no CUPS

1. Acesse o painel do CUPS:

```
http://localhost:631
```

2. Vá em **Administration > Add Printer**

3. Selecione sua impressora Epson TM-T20II (USB ou rede)

4. Escolha um driver genérico (ex: Raw ou Generic ESC/POS)

5. Nome sugerido para a impressora:

```
Epson_TM_T20II
```

6. Finalize a configuração e imprima uma página de teste.

---

## 🧪 Alternativa via Terminal

Você pode configurar a impressora diretamente:

```bash
lpadmin -p Epson_TM_T20II -E -v usb://EPSON/TM-T20II -m raw
```

> O filtro `rastertozj` será usado automaticamente se estiver instalado corretamente.

---

## ✅ Observações

- Este projeto **não depende de arquivos `.ppd`**
- Compatível com CUPS moderno e preparado para ambientes sem suporte a PPDs
- Ideal para sistemas embarcados, PDVs e automações com impressão térmica

---

## 📦 Licença

Este projeto é fornecido como está, sem garantias. Uso e modificação são livres conforme os termos da licença original.
