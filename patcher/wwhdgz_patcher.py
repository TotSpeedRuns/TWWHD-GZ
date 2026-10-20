"""WWHDGZ native copy-and-patch desktop application."""
import argparse,json,sys,threading,queue
from pathlib import Path
from folder_engine import create_copy,TEST_TITLE,resolve_game_folder
from patch_engine import patch_rpx
from package_engine import package_install,read_common_key

def resources():return Path(getattr(sys,'_MEIPASS',Path(__file__).parent))
def tools_resources():
 # Larger portable Java tools stay beside the executable, not in its temp unpack.
 return Path(sys.executable).parent if getattr(sys,'frozen',False) else Path(__file__).parent
def build(source,output,key_file=None,progress=lambda message,fraction:None,cancel=lambda:False):
 if key_file:
  read_common_key(key_file)
  install=Path(output).with_name(Path(output).name+' Install')
  if install.exists() or install.is_symlink():raise ValueError('The install-output folder already exists. Choose a new output name.')
 report=create_copy(source,output,resources(),progress,cancel)
 if key_file:
  install=Path(output).with_name(Path(output).name+' Install')
  try:
   report['install']=package_install(output,install,tools_resources(),key_file,progress)
   report['install_output']=str(install)
  except Exception as error:
   raise RuntimeError(f'The verified game copy was created at {output}, but install packing failed: {error}') from error
 return report

def gui():
 import tkinter as tk
 from tkinter import ttk,filedialog,messagebox
 from PIL import Image,ImageTk
 root=tk.Tk();root.title('WWHDGZ Native Patcher');root.geometry('980x680');root.minsize(940,660)
 bg='#141e20';card='#203034';text='#edf1e9';muted='#a9bbb8';accent='#c7e771'
 root.configure(bg=bg)
 style=ttk.Style();style.theme_use('clam')
 style.configure('Native.TFrame',background=bg);style.configure('Card.TFrame',background=card)
 style.configure('Native.TLabel',background=bg,foreground=text,font=('Segoe UI',11))
 style.configure('Muted.TLabel',background=bg,foreground=muted,font=('Segoe UI',10))
 style.configure('Native.TEntry',fieldbackground='#304347',foreground=text,padding=9,font=('Segoe UI',10))
 style.configure('Native.TButton',background='#304347',foreground=text,font=('Segoe UI',10),padding=(14,8))
 style.map('Native.TButton',background=[('active','#415c61')])
 style.configure('Build.TButton',background=accent,foreground='#172324',font=('Segoe UI Semibold',11),padding=(20,11))
 style.map('Build.TButton',background=[('active','#ddf49e'),('disabled','#4c5b4a')])
 style.configure('Native.TCheckbutton',background=bg,foreground=text,font=('Segoe UI',10))
 style.configure('Native.Horizontal.TProgressbar',background=accent,troughcolor=card,borderwidth=0)
 side=tk.Frame(root,bg=card,width=210);side.pack(side='left',fill='y');side.pack_propagate(False)
 icon=ImageTk.PhotoImage(Image.open(resources()/'home-menu-icon-original.jpg').resize((112,112),Image.Resampling.LANCZOS));root.icon_asset=icon
 tk.Label(side,image=icon,bg=card).pack(pady=(38,18))
 tk.Label(side,text='WWHDGZ',font=('Segoe UI Semibold',24),fg=text,bg=card).pack()
 tk.Label(side,text='NATIVE PATCHER',font=('Segoe UI',10),fg=accent,bg=card).pack(pady=(2,36))
 for title,body in [('01  SELECT','Your original EUR v0 dump'),('02  BUILD','A separate modified copy'),('03  TEST','Wii U GamePad / Pro Controller')]:
  tk.Label(side,text=title,font=('Segoe UI Semibold',11),fg=text,bg=card,anchor='w').pack(fill='x',padx=22,pady=(12,4))
  tk.Label(side,text=body,font=('Segoe UI',9),fg=muted,bg=card,anchor='w',wraplength=175,justify='left').pack(fill='x',padx=22)
 tk.Label(side,text='EXPERIMENTAL 0.1\nHardware validation starts here.',font=('Segoe UI',9),fg=muted,bg=card,justify='left').pack(side='bottom',pady=28)
 main=ttk.Frame(root,style='Native.TFrame',padding=(30,25));main.pack(side='right',fill='both',expand=True)
 ttk.Label(main,text='A separate game. Your original stays intact.',style='Muted.TLabel').pack(anchor='w')
 tk.Label(main,text='Build your native practice ROM',font=('Segoe UI Semibold',23),fg=text,bg=bg,anchor='w').pack(fill='x',pady=(8,5))
 ttk.Label(main,text='Practice tools embedded in the game, with your custom Wii U Menu icon.',style='Muted.TLabel',wraplength=680).pack(anchor='w',pady=(0,22))
 source=tk.StringVar();output=tk.StringVar();key=tk.StringVar();want_install=tk.BooleanVar(value=False)
 busy=[False];cancel_event=threading.Event();messages=queue.Queue();result_path=[None]
 def row(label,variable,choose):
  ttk.Label(main,text=label,style='Native.TLabel').pack(anchor='w',pady=(10,5))
  frame=ttk.Frame(main,style='Native.TFrame');frame.pack(fill='x')
  ttk.Entry(frame,textvariable=variable,style='Native.TEntry').pack(side='left',fill='x',expand=True)
  ttk.Button(frame,text='Browse',style='Native.TButton',command=choose).pack(side='right',padx=(10,0))
 def choose_source():
  path=filedialog.askdirectory(title='Select original WWHD game folder (code, content, meta)')
  if path:
   try:path=str(resolve_game_folder(path))
   except ValueError as error:messagebox.showerror('Choose extracted game folder',str(error));return
   source.set(path)
   if not output.get():output.set(str(Path(path).with_name(Path(path).name+' - WWHDGZ')))
 def choose_output():
  parent=filedialog.askdirectory(title='Select parent folder for the new modified game')
  if parent:output.set(str(Path(parent)/'WWHDGZ Native Test'))
 def choose_key():
  path=filedialog.askopenfilename(title='Select your dumped Wii U common-key file',filetypes=[('Key/text files','*.key *.txt'),('All files','*')])
  if path:key.set(path)
 row('Original game folder',source,choose_source)
 ttk.Label(main,text='Select the extracted folder with code, content and meta. WUD/WUX/WUA files are not supported yet.',style='Muted.TLabel',wraplength=660).pack(anchor='w',pady=(5,0))
 row('New modified game folder',output,choose_output)
 ttk.Checkbutton(main,text='Also create Wii U Menu install files',variable=want_install,style='Native.TCheckbutton').pack(anchor='w',pady=(17,2))
 row('Wii U common-key file (only needed for install files)',key,choose_key)
 ttk.Label(main,text='The key stays on your PC and is omitted from reports. Requires a homebrew-enabled Wii U.',style='Muted.TLabel',wraplength=660).pack(anchor='w',pady=(5,12))
 status=tk.StringVar(value='Ready · EUR v0 only · separate title '+TEST_TITLE.upper())
 bar=ttk.Progressbar(main,style='Native.Horizontal.TProgressbar',maximum=100);bar.pack(fill='x',pady=(8,8))
 ttk.Label(main,textvariable=status,style='Muted.TLabel',wraplength=660).pack(anchor='w',fill='x')
 def progress(message,fraction):messages.put(('progress',message,fraction))
 def worker(src,dst,k):
  try:messages.put(('done',build(src,dst,k,progress,cancel_event.is_set)))
  except Exception as e:messages.put(('error',str(e)))
 def start():
  if busy[0]:return
  if not source.get() or not output.get():messagebox.showerror('Choose game and output','Select your original game folder and a new output folder.');return
  if want_install.get() and not key.get():messagebox.showerror('Key file required','Select your dumped Wii U common-key file to create install files. You can create the modified game folder without it.');return
  busy[0]=True;cancel_event.clear();button.configure(state='disabled');status.set('Checking source and output…');bar['value']=0
  threading.Thread(target=worker,args=(source.get(),output.get(),key.get() if want_install.get() else None),daemon=True).start()
 def poll():
  try:
   while True:
    m=messages.get_nowait()
    if m[0]=='progress':status.set(m[1]);bar['value']=m[2]*100
    elif m[0]=='done':
     busy[0]=False;button.configure(state='normal');bar['value']=100;result_path[0]=output.get()
     status.set('Created and verified. Original untouched. Native console testing is still needed.')
     messagebox.showinfo('Native test copy created','Created:\n'+output.get()+('\n\nInstall files:\n'+m[1]['install_output'] if 'install_output' in m[1] else '')+'\n\nThe original was not changed. See the build report in the new folder.')
    elif m[0]=='error':busy[0]=False;button.configure(state='normal');status.set(m[1]);messagebox.showerror('Build stopped',m[1])
  except queue.Empty:pass
  root.after(100,poll)
 footer=ttk.Frame(main,style='Native.TFrame');footer.pack(fill='x',side='bottom',pady=(12,0))
 button=ttk.Button(footer,text='Create modified game',style='Build.TButton',command=start);button.pack(side='right')
 ttk.Label(footer,text='No game files are included.\nExisting output folders are never overwritten.',style='Muted.TLabel').pack(side='left')
 def close():
  if busy[0]:messagebox.showinfo('Build running','The copy is still being verified. Wait for it to finish before closing.');return
  root.destroy()
 root.protocol('WM_DELETE_WINDOW',close);root.after(100,poll);root.mainloop()

def main():
 p=argparse.ArgumentParser();p.add_argument('--source');p.add_argument('--output');p.add_argument('--report');p.add_argument('--check',action='store_true');p.add_argument('--common-key-file')
 a=p.parse_args()
 if not a.source:return gui()
 try:
  if a.check:
   _,report=patch_rpx((resolve_game_folder(a.source)/'code/cking.rpx').read_bytes(),resources()/'payload');report['read_only_check']=True
  else:
   if not a.output:raise ValueError('--output is required for a new modified game copy.')
   report=build(a.source,a.output,a.common_key_file)
 except Exception as e:
  report={'passed':False,'error':str(e)}
  if a.report:Path(a.report).write_text(json.dumps(report,indent=2))
  return 1
 if a.report:Path(a.report).write_text(json.dumps(report,indent=2))
 return 0
if __name__=='__main__':sys.exit(main())
