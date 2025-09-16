# Solo Seguro

Sistema de monitoramento de umidade do solo utilizando ESP32 e sensor de umidade capacitivo.

## Sobre o Projeto

O Solo Seguro é um sistema IoT que monitora a umidade do solo em tempo real. Ele cria uma rede Wi-Fi própria e disponibiliza uma interface web para visualização dos dados.

## Componentes Necessários

- ESP32 (qualquer modelo)
- Sensor de umidade do solo capacitivo v1.2
- Cabos jumper
- Fonte de alimentação para o ESP32 (USB ou bateria)

## Conexões

- **Sensor de umidade (saída digital)**: GPIO 26
- **Sensor de umidade (saída analógica)**: GPIO 34

## Funcionalidades

- Leitura digital (seco/úmido) e analógica (porcentagem) da umidade do solo
- Interface web responsiva com atualizações em tempo real
- Indicadores visuais do estado do solo (seco, moderado, úmido)
- Barra de progresso colorida
- Botão para pausar/retomar atualizações
- Exibição do valor bruto do sensor

## Como Usar

1. Carregue o código para o ESP32 usando a Arduino IDE
2. Conecte o sensor de umidade conforme as instruções acima
3. Alimente o ESP32
4. Conecte-se à rede Wi-Fi "Solo Seguro" (senha: soloseguro123)
5. Acesse o endereço IP 192.168.4.1 em qualquer navegador
6. Visualize os dados de umidade do solo em tempo real

## Calibração

O sistema vem pré-calibrado com os seguintes valores:
- Ar (seco): 3200
- Água (úmido): 1400

Para melhor precisão, você pode ajustar esses valores no código de acordo com as leituras do seu sensor específico.

## Personalização

Você pode personalizar:
- Nome e senha da rede Wi-Fi
- Valores de calibração
- Intervalos de atualização
- Aparência da interface web

## Solução de Problemas

- **Leituras inconsistentes**: Verifique as conexões do sensor e os valores de calibração
- **Rede Wi-Fi não aparece**: Reinicie o ESP32 e verifique a alimentação
- **Interface web não carrega**: Verifique se está conectado à rede correta e tente acessar o IP 192.168.4.1

## Próximas Melhorias

- Adicionar suporte para sensor de temperatura DS18B20 (requer resistor pull-up de 4.7kΩ)
- Implementar registro de dados históricos
- Adicionar configuração via interface web
- Suporte para conexão com redes Wi-Fi existentes 