# Epson TM-T20II

CUPS filter for thermal printer Epson TM-T20II

O driver oficial da Epson para Linux não funciona em sistemas ARM como o Raspberry Pi, pois é um binário compilado para arquitetura x86.

Este projeto fornece um filtro CUPS (`rastertozj`) compatível com ARM, permitindo impressão térmica na Epson TM-T20II. Também otimiza o tratamento de linhas em branco para melhorar a performance.

---

## 🔧 Requisitos para Compilação

Para compilar o filtro em Raspberry Pi ou qualquer sistema baseado em Debian/Ubuntu com arquitetura ARM, instale os pacotes necessários:

```bash
sudo apt-get install libcups2-dev libcupsimage2-dev g++ cups cups-client
```

Depois, compile o filtro:

```bash
make
```

---

## 📁 Instalação Após a Compilação

Após a compilação bem-sucedida, instale o binário e o arquivo `.ppd` manualmente:

```bash
sudo cp rastertozj /usr/lib/cups/filter/
sudo chmod +x /usr/lib/cups/filter/rastertozj

sudo mkdir -p /usr/share/ppd/custom/
sudo cp tm20.ppd /usr/share/ppd/custom/
sudo chmod 644 /usr/share/ppd/custom/tm20.ppd
```

Em alguns sistemas, o caminho do filtro pode ser `/usr/libexec/cups/filter/`. Verifique com:

```bash
cups-config --serverbin
```

---

## 🖨️ Configuração no CUPS

Para configurar a impressora:

1. Acesse a interface web do CUPS:

```
http://localhost:631
```

Ou substitua `localhost` pelo IP do Raspberry se estiver acessando remotamente.

2. Vá em **Administration > Add Printer**

3. Selecione sua impressora Epson TM-T20II (USB ou rede)

4. Quando solicitado o driver:

- Escolha **"Provide a PPD file manually"**
- Selecione: `/usr/share/ppd/custom/tm20.ppd`

5. Nome sugerido para a impressora:

```
Epson_TM_T20II
```

6. Finalize a configuração e imprima uma página de teste.

---

## 🧪 Alternativa via Terminal

Você também pode configurar a impressora diretamente pelo terminal:

```bash
lpadmin -p Epson_TM_T20II -E -v usb://EPSON/TM-T20II -P /usr/share/ppd/custom/tm20.ppd
```

- `-p Epson_TM_T20II`: nome da impressora
- `-E`: ativa a impressora
- `-v`: caminho da impressora (verifique com `lpinfo -v`)
- `-P`: caminho completo do arquivo `.ppd`

---

## ✅ Observações

- Testado com sucesso em Raspberry Pi OS e Ubuntu Server ARM64.
- Os avisos de funções obsoletas do CUPS durante a compilação podem ser ignorados — o filtro continua funcional.
- Para impressão em rede ou integração com sistemas de PDV, configurações adicionais podem ser necessárias.

---

## 📦 Licença

Este projeto é fornecido como está, sem garantias. Uso e modificação são livres conforme os termos da licença original.
