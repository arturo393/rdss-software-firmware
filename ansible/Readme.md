# RDSS Ansible miniPC installation

## Local Dependencies

Install sshpass àpt get install sshpass`

Install ansible locally

Please refer to:
[Installing Ansible on specific operating systems](https://docs.ansible.com/ansible/latest/installation_guide/intro_installation.html#installing-ansible-on-specific-operating-systems)

## Setting up deploy environment

Edit `vars.yaml` file and change variables with yours

Define templates located at `playbooks/templates` as you desire

## Add all miniPCs to inventory

edit `inventory/hosts.yaml` file and add all miniPCs IPs
The content should be something like:

```
miniPCs:
  hosts:
    miniPC1:
      ansible_host: 165.227.192.32
    miniPC2:
      ansible_host: 192.168.60.79
    miniPC3:
      ansible_host: 192.168.60.80
```

## Provisioning miniPCs

Just execute:

`ansible-playbook -K main.yaml --ask-pass`

(will be prompted miniPC user sudo password)
