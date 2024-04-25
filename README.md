# Software RDSS v2.x


### Documentación y Requisitos
- [Arreglar Documento de requerimientos para sea general](https://www.notion.so/Arreglar-Documento-de-requermientos-para-sea-general-7e08d3bc8b4244a6b41a1bc55eef0d91?pvs=21)
- [Documentación BackEnd](https://www.notion.so/Documentaci-n-BackEnd-4561c4adc66b40bfaaeed080a5d1b164?pvs=21)
- [Documentación - Base de datos](https://www.notion.so/Documentaci-n-Base-de-datos-74a228434b0b408db233129ea330d3a8?pvs=21)
- [Documentación - Descripción General](https://www.notion.so/Documentaci-n-Descripci-n-General-f9b53fe77d8645108d949d5f8ae4160b?pvs=21)
- [Documentación - FrontEnd](https://www.notion.so/Documentaci-n-FrontEnd-0350f64d83e44570986191dbdd66333d?pvs=21)
- [Documento de Requerimientos para Itaum](https://www.notion.so/Documento-de-Requerimientos-para-Itaum-0fbd3f52b18045cdb0899152e09f4975?pvs=21)

### Desarrollo y Pruebas
- [Evaluar desarrollo de nuevas características VLAD con itaum](https://www.notion.so/Evaluar-desarrollo-de-nuevas-caracter-sticas-VLAD-con-itaum-08bca6cdb09b46d08a4aa6362a32adad?pvs=21)
- [Ajuste de los voltajes de salida](https://www.notion.so/Ajuste-de-los-voltajes-de-salida-13a4fb37fa8343b0a68dd97535a67285?pvs=21)
- [Instalación de software en servidor de prueba](https://www.notion.so/Instalaci-n-de-software-en-servidor-de-prueba-47844e4c8b814b9eafb3e19b88c27171?pvs=21)
- [Revisar valores de corriente según datasheet](https://www.notion.so/Revisar-valores-de-corriente-seg-n-datasheet-98180151f3ed47b5b08bea72c2fa64ea?pvs=21)
- [Revisar porque no actualiza el SmartTune en 192.168.60.236](https://www.notion.so/Revisar-porque-no-actualiza-el-SmartTune-en-192-168-60-236-7101a2665f28413281087bde99b9ee00?pvs=21)
- [Revisar mapeo de valores AGC](https://www.notion.so/Revisar-mapeo-de-valores-AGC-e1beb48a0457421696622d07ae44c888?pvs=21)
- [Test Demo RDSS](https://www.notion.so/Test-Demo-RDSS-e7467e9d35df469bb81980a6ae51eb27?pvs=21)

### Implementación y Configuración
- [Agregrar condición de valores ceros en monitor.py](https://www.notion.so/Agregrar-condici-n-de-valores-ceros-en-monitor-py-32f6544ca6e041a4886f35e19c86f874?pvs=21)
- [Agregar configuración de usuario sigmadev en documentación](https://www.notion.so/Agregar-configuraci-n-de-usuario-sigmadev-en-doncumentaci-n-92e6522b31854917b8b137686337f910?pvs=21)
- [Agregar nueva decodificación de consulta VLAD](https://www.notion.so/Agregar-nueva-decodificaci-n-de-consulta-VLAD-a15334c95f2f4684aab7d111f489e548?pvs=21)
- [Cambiar el nombre de attenuation downlink a attenuation](https://www.notion.so/Cambiar-el-nombre-de-attenuation-downlink-a-attenuation-7d9e808035c04137ac671b98be800a08?pvs=21)

### Gestión y Organización
- [Revisión General ChatGPT](https://www.notion.so/Revisi-n-General-ChatGPT-9fd2b14427ed492ea4e03ef0fc7a2add?pvs=21)
- [Orderna Documento](https://www.notion.so/Orderna-Documento-635e6b61664c40e7a8129c128b214084?pvs=21)
- [Limites de parametros de alertas](https://www.notion.so/Limites-de-parametros-de-alertas-ba795d5e3227439aad55cec5512fcba4?pvs=21)
- [Se carga software en 192.168.60.75](https://www.notion.so/Se-carga-software-en-192-168-60-75-5dd02affb6c7428993a294d3ac0f910a?pvs=21)
- [Enviar consulta de master](https://www.notion.so/Enviar-consulta-de-master-82f2f99efbe842f0a504b769454f5d23?pvs=21)
- [Ordernar el log del monuitor](https://www.notion.so/Ordernar-el-log-del-monuitor-187a1ca22fc443e4816dd0ef3613b057?pvs=21)

### Otros
- [Inicio. ver documento: Pruebas de Compatibilidad Diagnostico Remoto Tarjeta Azul con Software](https://www.notion.so/Inicio-ver-documento-Pruebas-de-Compatibilidad-Diagnostico-Remoto-Tarjeta-Azul-con-Software-871e7544f5fc4dff83d035a36b671509?pvs=21)
- [Finalización. ver documento: Pruebas de Compatibilidad Diagnostico Remoto Tarjeta Azul con Software](https://www.notion.so/Finalizaci-n-ver-documento-Pruebas-de-Compatibilidad-Diagnostico-Remoto-Tarjeta-Azul-con-Software-56f464418c2749debde48d09b06cbc64?pvs=21)

### Pruebas y Validación
- [versión de prueba decode y encode de sniffer](https://www.notion.so/versi-n-de-prueba-decode-y-encode-de-sniffer-c7919068be784ce3810b24a3257df495?pvs=21)
- [code y decode de texto](https://www.notion.so/code-y-decode-de-texto-b1fa37f3009a4e71a0596d11899f377e?pvs=21)
- [cotización frontend yery](https://www.notion.so/cotizaci-n-frontend-yery-ddf0d7d4f8b74e4e8d9dfcbaaaba1cff?pvs=21)


**Informe sobre Ajustes en Mediciones del VLAD**

---


Aquí presento un resumen de los ajustes realizados en las mediciones del VLAD, junto con los detalles de los errores cuadráticos medios calculados para cada medida:

- **Voltaje:** Error de 0.003 [V]
- **Corriente:** Error de 0.00001 [A]
- **Potencia de Downlink:** Error de 0.07 [dBm]
- **AGC de Downlink y Uplink:** Error de 0.7 [dB]

Estos ajustes se llevaron a cabo utilizando una fuente de poder y un analizador de espectro para ajustar los valores de corriente, voltaje, potencia y AGC. Es esencial tener en cuenta que estos ajustes se realizaron para el atenuador en una posición específica, con una ganancia de 20 [dB].

Sin embargo, se identificó una limitación en la detección de la atenuación a través del switch rotatorio en la placa de diagnóstico remoto actual. Esto implica que, si se modifica la atenuación, el valor de la potencia de downlink no reflejará el valor real.

Estoy disponible para cualquier aclaración o duda adicional que puedas tener.

---

**Configuración de Red de Laboratorio**

![Configuración de Red de Laboratorio](Software%20RDSS%20v2%20x%2030054446c92343aaa14b0927fa4a5c08/Untitled.png)

---

Este informe está relacionado con las tareas anteriores en el desarrollo del software de diagnóstico remoto.

[Enlace al documento completo](https://docs.google.com/document/d/1h1PHycrrpae6Wp-V5XpYpGTlTFufmnP5c2jyUwDU6aM/edit#)