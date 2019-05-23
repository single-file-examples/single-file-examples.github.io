import sublime
import sublime_plugin
import os


class SingleFileExampleBuildCommand(sublime_plugin.TextCommand):
    def run(self, edit):
        if len(self.view.file_name()) > 0:
            lines = self.view.find_all(r"(\s*\s+g\+\+[^\r\n]+)")
            file = os.path.split(self.view.file_name())

            for region in lines:
                command = self.view.substr(region).lstrip()
                command = command.replace('g++', 'g++ ' + file[1])

                print('running '+self.view.substr(region))
                exec_options = {
                    'env': {
                        'PATH': '$MINGW_BIN;$PATH'
                    },
                    'quiet': False,
                    'working_dir': file[0],
                    'shell_cmd': command + ' -o ' + file[1] + '.exe'
                }
                sublime.active_window().run_command('exec', exec_options)

class SingleFileExampleRunCommand(sublime_plugin.TextCommand):
    def run(self, edit):
        if len(self.view.file_name()) > 0:
            file = os.path.split(self.view.file_name())

            exec_options = {
                'env': {
                    'PATH': '$MINGW_BIN;$PATH'
                },
                'quiet': False,
                'working_dir': file[0],
                'shell_cmd': file[1] + '.exe'
            }
            sublime.active_window().run_command('exec', exec_options)
