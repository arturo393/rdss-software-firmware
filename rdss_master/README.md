<<<<<<< HEAD
# rdss



## Getting started

To make it easy for you to get started with GitLab, here's a list of recommended next steps.

Already a pro? Just edit this README.md and make it your own. Want to make it easy? [Use the template at the bottom](#editing-this-readme)!

## Add your files

- [ ] [Create](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#create-a-file) or [upload](https://docs.gitlab.com/ee/user/project/repository/web_editor.html#upload-a-file) files
- [ ] [Add files using the command line](https://docs.gitlab.com/ee/gitlab-basics/add-file.html#add-a-file-using-the-command-line) or push an existing Git repository with the following command:

```
cd existing_repo
git remote add origin https://gitlab.com/sigma-telecom/rdss.git
git branch -M main
git push -uf origin main
```

## Integrate with your tools

- [ ] [Set up project integrations](https://gitlab.com/sigma-telecom/rdss/-/settings/integrations)

## Collaborate with your team

- [ ] [Invite team members and collaborators](https://docs.gitlab.com/ee/user/project/members/)
- [ ] [Create a new merge request](https://docs.gitlab.com/ee/user/project/merge_requests/creating_merge_requests.html)
- [ ] [Automatically close issues from merge requests](https://docs.gitlab.com/ee/user/project/issues/managing_issues.html#closing-issues-automatically)
- [ ] [Enable merge request approvals](https://docs.gitlab.com/ee/user/project/merge_requests/approvals/)
- [ ] [Set auto-merge](https://docs.gitlab.com/ee/user/project/merge_requests/merge_when_pipeline_succeeds.html)

## Test and Deploy

Use the built-in continuous integration in GitLab.

- [ ] [Get started with GitLab CI/CD](https://docs.gitlab.com/ee/ci/quick_start/index.html)
- [ ] [Analyze your code for known vulnerabilities with Static Application Security Testing (SAST)](https://docs.gitlab.com/ee/user/application_security/sast/)
- [ ] [Deploy to Kubernetes, Amazon EC2, or Amazon ECS using Auto Deploy](https://docs.gitlab.com/ee/topics/autodevops/requirements.html)
- [ ] [Use pull-based deployments for improved Kubernetes management](https://docs.gitlab.com/ee/user/clusters/agent/)
- [ ] [Set up protected environments](https://docs.gitlab.com/ee/ci/environments/protected_environments.html)

***

# Editing this README

When you're ready to make this README your own, just edit this file and use the handy template below (or feel free to structure it however you want - this is just a starting point!). Thanks to [makeareadme.com](https://www.makeareadme.com/) for this template.

## Suggestions for a good README

Every project is different, so consider which of these sections apply to yours. The sections used in the template are suggestions for most open source projects. Also keep in mind that while a README can be too long and detailed, too long is better than too short. If you think your README is too long, consider utilizing another form of documentation rather than cutting out information.

## Name
Choose a self-explaining name for your project.

## Description
Let people know what your project can do specifically. Provide context and add a link to any reference visitors might be unfamiliar with. A list of Features or a Background subsection can also be added here. If there are alternatives to your project, this is a good place to list differentiating factors.

## Badges
On some READMEs, you may see small images that convey metadata, such as whether or not all the tests are passing for the project. You can use Shields to add some to your README. Many services also have instructions for adding a badge.

## Visuals
Depending on what you are making, it can be a good idea to include screenshots or even a video (you'll frequently see GIFs rather than actual videos). Tools like ttygif can help, but check out Asciinema for a more sophisticated method.

## Installation
Within a particular ecosystem, there may be a common way of installing things, such as using Yarn, NuGet, or Homebrew. However, consider the possibility that whoever is reading your README is a novice and would like more guidance. Listing specific steps helps remove ambiguity and gets people to using your project as quickly as possible. If it only runs in a specific context like a particular programming language version or operating system or has dependencies that have to be installed manually, also add a Requirements subsection.

## Usage
Use examples liberally, and show the expected output if you can. It's helpful to have inline the smallest example of usage that you can demonstrate, while providing links to more sophisticated examples if they are too long to reasonably include in the README.

## Support
Tell people where they can go to for help. It can be any combination of an issue tracker, a chat room, an email address, etc.

## Roadmap
If you have ideas for releases in the future, it is a good idea to list them in the README.

## Contributing
State if you are open to contributions and what your requirements are for accepting them.

For people who want to make changes to your project, it's helpful to have some documentation on how to get started. Perhaps there is a script that they should run or some environment variables that they need to set. Make these steps explicit. These instructions could also be useful to your future self.

You can also document commands to lint the code or run tests. These steps help to ensure high code quality and reduce the likelihood that the changes inadvertently break something. Having instructions for running tests is especially helpful if it requires external setup, such as starting a Selenium server for testing in a browser.

## Authors and acknowledgment
Show your appreciation to those who have contributed to the project.

## License
For open source projects, say how it is licensed.

## Project status
If you have run out of energy or time for your project, put a note at the top of the README saying that development has slowed down or stopped completely. Someone may choose to fork your project or volunteer to step in as a maintainer or owner, allowing your project to keep going. You can also make an explicit request for maintainers.
=======
# Resumen del Código

El código de este firmware se enfoca en el desarrollo de un maestro de comunicación por LoRa, implementando diversas mejoras, pruebas y características. A continuación se detallan las tareas realizadas durante el desarrollo:

### Version 2.0

https://drive.google.com/file/d/1n2ZLFueURxkfg9BF6pYhfqQwA9h9KJky/view?usp=drive_link

https://drive.google.com/file/d/1n4RY4IEEJZgvGGrLIxynTIYqDLopjHAY/view?usp=sharing

Nue

Para el Master Uqomm:

- Compatibilidad con LTEL.
- Consulta y ajuste de parámetros locales y de esclavos.
- Protocolo de comunicación Uqomm.
- Consulta de temperatura, voltaje de alimentación y corriente de consumo mediante RS485.
- Consulta de parámetros de VLAD mediante LoRa.

Para el Esclavo Uqomm:

- Compatibilidad con LTEL.
- Consulta y ajuste de parámetros locales a través del Master.
- Protocolo de comunicación Uqomm.
- Obtención de parámetros de VLAD mediante I2C.
- Consulta de parámetros de VLAD mediante RS232.
- Envío de parámetros de VLAD mediante LoRa.

[https://drive.google.com/file/d/1n4RY4IEEJZgvGGrLIxynTIYqDLopjHAY/view?usp=sharing](https://drive.google.com/file/d/1n4RY4IEEJZgvGGrLIxynTIYqDLopjHAY/view?usp=sharing)


## Mejoras:
- [Mejora en la comunicación i2c por interrupción para mejorar la comunicación](https://www.notion.so/Mejora-en-la-comunicaci-n-i2c-por-interrupci-n-para-mejorar-la-comunicaci-n-0489ad60379d465198a1517431b780dc?pvs=21)
- [Limpiar Driver UART1](https://www.notion.so/Limpiar-Driver-UART1-0db5ff62227e4a0c85c4c770df40b506?pvs=21)
- [Implementación de nuevos vlad para pruebas](https://www.notion.so/Implementaci-n-de-nuevos-vlad-para-pruebas-1773210a4d8341b99d2c9f686b4fa683?pvs=21)
- [Limpieza de código rdss_slave](https://www.notion.so/Limpieza-de-c-digo-rdss_slave-25a2b5302cc24bc3bde693a4bca65ddf?pvs=21)
- [Limpiar Driver SX1278](https://www.notion.so/Limpiar-Driver-SX1278-40f1ee8f71504b8394cdacf41772eeb3?pvs=21)
- [Driver SX1278 → Bajar el uso de memoria](https://www.notion.so/Driver-SX1278-Bajar-el-uso-de-memoria-5efdd1799f97416f8b72ba22bd6eb259?pvs=21)
- [Disminución de memoria](https://www.notion.so/Disminuci-n-de-memoria-600a69e1cc6c4531b82250d0136fd7cf?pvs=21)
- [Mejora de i2c para el esclavo vladr](https://www.notion.so/Mejora-de-i2c-para-el-esclavo-vladr-bc52f5d860744462a68ec2d67c7a649d?pvs=21)
- [Driver SX1278 → Reiniciar puntero de del FIFO de recepción](https://www.notion.so/Driver-SX1278-Reiniciar-puntero-de-del-FIFO-de-recepci-n-8c67b93814434e90b45966008e1d71d3?pvs=21)
- [I2c Master Bare Metal](https://www.notion.so/I2c-Master-Bare-Metal-e33861f3c75f42649e40bdedfab7d01e?pvs=21)

## Pruebas:
- [Revisión completa con señal en red real](https://www.notion.so/Revisi-n-completa-con-se-al-en-red-real-1c980d7b17984a80b122d2a34ba019b4?pvs=21)
- [Pruebas - Protocolo de Comunicación Driver Master / Slave](https://www.notion.so/Pruebas-Protocolo-de-Comunicaci-n-Driver-Master-Slave-8012cbccd5b84dd1ac35caa1832beba5?pvs=21)
- [Pruebas de comunicación](https://www.notion.so/Pruebas-de-comunicaci-n-c00db88e5371474fa8ae17afb2607eef?pvs=21)

## Documentación:
- [Documentación de prueba de envío de comandos Master → Esclavo → Master](https://www.notion.so/Documentaci-n-de-prueba-de-env-o-de-comandos-Master-Esclavo-Master-3bd95e6f55ae46bc83e2782ee17b0268?pvs=21)
- [Agregar diagrama de comunicasción entre vlad, sclavo, master, software](https://www.notion.so/Agregar-diagrama-de-comunicasci-n-entre-vlad-sclavo-master-software-d6393606de8f4ea2b99f89b4025c558d?pvs=21)

## Características:
- [Enviar datos sin decodificar y decodificar en el software](https://www.notion.so/Enviar-datos-sin-decodificar-y-decodificar-en-el-software-0c12c6941fd44d1e8d204f9f08f8f010?pvs=21)
- [Enviar parametros por uart con query 0x11 para el software chino](https://www.notion.so/Enviar-parametros-por-uart-con-query-0x11-para-el-software-chino-2061e79ea19e48cdaa0a28c6a2cc52d2?pvs=21)
- [Esclavo Uqomm → Guardar y leer parametros LoRa en Memoria](https://www.notion.so/Esclavo-Uqomm-Guardar-y-leer-parametros-LoRa-en-Memoria-2451b052aea84f49881b084050f28928?pvs=21)
- [Esclavo Uqomm → Protocolo de comunicación](https://www.notion.so/Esclavo-Uqomm-Protocolo-de-comunicaci-n-387608fb720d4e1b8425d7afca88fd52?pvs=21)
- [Master Uqomm → Driver HAL UART + RS485](https://www.notion.so/Master-Uqomm-Driver-HAL-UART-RS485-308ee6c92e894fed8ff3b854b1295241?pvs=21)
- [Master Uqomm → Driver UART1 uqomm](https://www.notion.so/Master-Uqomm-Driver-UART1-uqomm-443993f635044e72a497c1f7deb786aa?pvs=21)
- [Esclavo Uqomm → Driver I2C Master](https://www.notion.so/Esclavo-Uqomm-Driver-I2C-Master-28cc03fc93cb4f95b2a558c5a9ca1abb?pvs=21)
- [Esclavo Uqomm → Driver RS485 + UART](https://www.notion.so/Esclavo-Uqomm-Driver-RS485-UART-cc4961edaf214cd1bda2146820e6eca4?pvs=21)
- [Driver LM745](https://www.notion.so/Master-Uqomm-Driver-lm745-fa0c364ba1c94f0d94af4a1ce54905ab?pvs=21)
- [Driver LM750](https://www.notion.so/Master-Uqomm-Driver-lm750-c9b4f5b8a9fc4331b539c9b51ad0a5de?pvs=21)
- [Driver RS485](https://www.notion.so/Driver-RS485-276f215839d3463faa16fe67fdd324b2?pvs=21)
- [Driver SPI](https://www.notion.so/Driver-SPI-656f07147fe9412fbbf1955e867da115?pvs=21)
- [Driver Timer](https://www.notion.so/Driver-Timer-7e6eeb19564e4d12ad6e7208c3048a6d?pvs=21)
- [Prueba de comunicación UART y RS485](https://www.notion.so/Prueba-de-comunicaci-n-UART-y-RS485-d7c08cdd91f441dbb12a5d76ec1a4dc3?pvs=21)
- [Recepción y almacenamiento en la memoria de los datos](https://www.notion.so/Recepci-n-y-almacenamiento-en-la-memoria-de-los-datos-c7206c6c7e484452869c01d76fbff91e?pvs=21)
- [Realización de configuración de la comunicación con software](https://www.notion.so/Realizaci-n-de-configuraci-n-de-la-comunicaci-n-con-software-7e3b0bbfdb5a4f39a2d3c52e3a94cc8b?pvs=21)

>>>>>>> repo-a/master
