# -*- mode: ruby -*-
# vi: set ft=ruby :

box  = 'trusty64'
url  = 'http://cloud-images.ubuntu.com/vagrant/trusty/current/trusty-server-cloudimg-amd64-vagrant-disk1.box'
name = 'tmbodev'
ssh_port = 2000

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  config.vm.define name, primary: true do |tmbodev|
    # Every Vagrant virtual environment requires a box to build off of.
    tmbodev.vm.box = box
    tmbodev.vm.box_url = url

    # Share SSH and http
    tmbodev.vm.network :forwarded_port, guest: 22, host: ssh_port, id: "ssh", auto_correct: true
    tmbodev.vm.network :forwarded_port, guest: 80, host: 8080
    # XXX: Share HTTPS eventually
    # tmbodev.vm.network :forwarded_port, guest: 80, host: 8080


    # Share an additional folder to the guest VM. The first argument is
    # the path on the host to the actual folder. The second argument is
    # the path on the guest to mount the folder. And the optional third
    # argument is a set of non-required options.
    tmbodev.vm.synced_folder ".", "/home/vagrant/sites/tmbo"
    # tmbodev.vm.synced_folder ".", "/vagrant", disabled: true

    # Provision using a shell script
    tmbodev.vm.provision :shell, path: "admin/vm_setup.sh"

    # Provider-specific configuration so you can fine-tune various
    # backing providers for Vagrant. These expose provider-specific options.
    # Example for VirtualBox:
    #
    tmbodev.vm.provider :virtualbox do |vb|
      # Customize the amount of memory on the VM:
      vb.memory = "512"

      # Display the VirtualBox GUI when booting the machine
      vb.gui = true

    #   # Use VBoxManage to customize the VM. For example to change memory:
    #   vb.customize ["modifyvm", :id, "--memory", "1024"]
    end

  end

  # Define a Vagrant Push strategy for pushing to Atlas. Other push strategies
  # such as FTP and Heroku are also available. See the documentation at
  # https://docs.vagrantup.com/v2/push/atlas.html for more information.
  # config.push.define "atlas" do |push|
  #   push.app = "YOUR_ATLAS_USERNAME/YOUR_APPLICATION_NAME"
  # end

  # Enable provisioning with a shell script. Additional provisioners such as
  # Puppet, Chef, Ansible, Salt, and Docker are also available. Please see the
  # documentation for more information about their specific syntax and use.
  # config.vm.provision "shell", inline: <<-SHELL
  #   sudo apt-get update
  #   sudo apt-get install -y apache2
  # SHELL
end
