# tmpl

A Template Directory Manager CLI Software (Windows only for now).


------------

### How it Works
Do you want to use your own templates on your projects simply by placing the name of your template in the Command Prompt? This is the right place !

With this tool, you can create templates for a specific folder and it will be saved by the program, so don't worry about changing your template folder after the template creation operation.

You can give any name to your template, for example do the following command on your folder for which you want to create a template: tmpl create hello.

Perfect ! You have created your Template! Now, if you want to get it, just go to a folder you want and write tmpl get hello: as you can see, the same folders and files will appear in the folder as the folder from which you created the template (obviously it will only collect the template data, not of the folder where you made the template, so any subsequent updates of the folder where you made the template will not be included).

**At the moment there are 9 types of commands to do on the Command Prompt:**

 > `tmpl create <template_name>` this creates a new template using the folder where you ran this command .
 
 > `tmpl get <template_name>` this uses a template you have already created, on the folder where you ran this command
 
 > `tmpl delete <template_name>` well... delete a Template you created!
 
 > `tmpl list` shows the list of templates you created.

 > `tmpl -v` shows the current version of tmpl installed on your computer.

 > `tmpl -u` check for updates on GitHub.

 > `tmpl import <template_input_file> [template_name]` import an external template that you can immediately use. In the template_name parameter, you must put the name you want to give to the template which, if left empty, will use the name of the Template File (another way to import a Template is to open the .tmpl file using this software.)

 > `tmpl export <template_name> <template_output_directory>` export your template to share it.

 > `tmpl help` show tmpl available commands (not really a command but it will still show you the info).


------------

## Contributions to the project are welcome!

