import argparse,json,sys,threading,queue,time
from pathlib import Path
from capture_log import checkpoint_name,named_snapshot,append_capture
from memfile_core import CemuMemory,CaptureError,capture,save_json,processes
from wiiu_memory import WiiUMemory

def gui(smoke_report=None,test_driver=None):
    import tkinter as tk
    from tkinter import ttk,filedialog,messagebox
    root=tk.Tk();root.title('WWHDGZ Memfile Creator 0.6');root.geometry('820x810');root.minsize(780,790)
    if smoke_report or test_driver:root.withdraw()
    frame=ttk.Frame(root,padding=20);frame.pack(fill='both',expand=True)
    ttk.Label(frame,text='WWHDGZ Memfile Creator',font=('Segoe UI',20,'bold')).pack(anchor='w')
    ttk.Label(frame,text='Named checkpoints from Cemu or Wii U • WWHD EUR base v0',font=('Segoe UI',10)).pack(anchor='w',pady=(3,10))
    state={'memory':None,'processes':[],'last':None,'busy':False,'generation':0,'next_poll':0};results=queue.Queue()
    selected=tk.StringVar();log=tk.StringVar();status=tk.StringVar(value='Choose Cemu or Wii U, load a playable save, then connect.')
    source=tk.StringVar(value='Cemu');ip=tk.StringVar();profile_address=tk.StringVar()
    row=ttk.Frame(frame);row.pack(fill='x',pady=5)
    ttk.Label(row,text='Capture from').pack(side='left')
    source_box=ttk.Combobox(row,textvariable=source,values=['Cemu','Wii U'],state='readonly',width=12);source_box.pack(side='left',padx=8)
    ttk.Label(row,text='Wii U IP').pack(side='left');ttk.Entry(row,textvariable=ip,width=20).pack(side='left',padx=8)
    ttk.Label(frame,text='Wii U: TCPGecko Aroma 7332 or legacy 7331 alongside WWHD-GZ. No hacker app required.',wraplength=750).pack(anchor='w',pady=(0,8))
    prow=ttk.Frame(frame);prow.pack(fill='x',pady=4)
    ttk.Label(prow,text='Creator profile number from GZ').pack(side='left')
    ttk.Entry(prow,textvariable=profile_address,width=20).pack(side='left',padx=8)
    ttk.Label(prow,text='Optional for older supported layouts').pack(side='left')
    combo=ttk.Combobox(frame,textvariable=selected,state='readonly');combo.pack(fill='x')
    row=ttk.Frame(frame);row.pack(fill='x',pady=8)
    def disconnect():
        state['generation']+=1;state['busy']=False
        if state['memory']:state['memory'].close()
        state['memory']=None;state['last']=None;capture_button.configure(state='disabled')
    def submit(operation,callback):
        if state['busy']:return
        state['busy']=True;generation=state['generation'];capture_button.configure(state='disabled')
        def worker():
            try:value=operation();error=None
            except Exception as e:value=None;error=e
            if generation!=state['generation']:
                if isinstance(value,WiiUMemory):value.close()
                return
            results.put((generation,callback,value,error))
        threading.Thread(target=worker,daemon=True).start()
    def refresh():
        disconnect()
        try:state['processes']=processes()
        except Exception as e:status.set(str(e));return
        combo['values']=[f'PID {pid} — {path}' for pid,path in state['processes']]
        if state['processes']:combo.current(0);status.set('Choose the Cemu instance and click Connect.')
        else:selected.set('');status.set('No running Cemu found.')
    def connect():
        disconnect()
        if source.get()=='Wii U':
            address=ip.get().strip();status.set('Connecting to Wii U '+address+' (Aroma / legacy)…')
            def connected(memory):
                state['memory']=memory;state['next_poll']=0;status.set('Connected to Wii U. Checking playable state…');poll(True)
            submit(lambda:WiiUMemory(address,profile_address=profile_address.get().strip() or None),connected);return
        index=combo.current()
        if index<0:status.set('Choose a running Cemu instance.');return
        try:
            state['memory']=CemuMemory(state['processes'][index][0],log.get().strip() or None)
            status.set('Connected. Waiting for a playable stage…');poll()
        except Exception as e:status.set(str(e))
    ttk.Button(row,text='Refresh Cemu list',command=refresh).pack(side='left')
    ttk.Button(row,text='Connect',command=connect).pack(side='left',padx=8)
    ttk.Button(row,text='Disconnect',command=lambda:(disconnect(),status.set('Disconnected.'))).pack(side='left')
    ttk.Button(row,text='Refresh game state',command=lambda:poll(True)).pack(side='left',padx=8)
    source_box.bind('<<ComboboxSelected>>',lambda e:(disconnect(),status.set('Choose the device and click Connect.')))
    ttk.Label(frame,text='Cemu log (automatic unless specified)').pack(anchor='w',pady=(8,0))
    row=ttk.Frame(frame);row.pack(fill='x');ttk.Entry(row,textvariable=log).pack(side='left',fill='x',expand=True)
    def browse():
        path=filedialog.askopenfilename(title='Choose this Cemu instance\'s log.txt',filetypes=[('Cemu log','*.txt'),('All files','*')])
        if path:log.set(path)
    ttk.Button(row,text='Browse log',command=browse).pack(side='left',padx=(8,0))
    name=tk.StringVar()
    ttk.Label(frame,text='Memfile name').pack(anchor='w',pady=(14,3))
    ttk.Entry(frame,textvariable=name).pack(fill='x')
    ttk.Label(frame,text='Example: TotG Before Command Melody').pack(anchor='w')
    exact=tk.BooleanVar(value=True);day=tk.BooleanVar(value=False)
    ttk.Checkbutton(frame,text='Restore exact position and facing when loading this checkpoint',variable=exact).pack(anchor='w',pady=(16,0))
    ttk.Checkbutton(frame,text='Require daytime when loading this checkpoint',variable=day).pack(anchor='w',pady=5)
    details=tk.StringVar(value='Stage —    Room —    Animation set —\nPosition —')
    ttk.Label(frame,textvariable=details,font=('Consolas',11)).pack(anchor='w',pady=12)
    def export():
        if not state['memory'] or state['busy']:return
        if isinstance(state['memory'],WiiUMemory):
            try:checkpoint_name(name.get())
            except Exception as e:messagebox.showerror('Memfile name',str(e));return
            memory=state['memory'];options=(exact.get(),day.get());value=name.get()
            status.set('Reading stable Wii U checkpoint… Stand still or pause gameplay.')
            submit(lambda:named_snapshot(capture(memory,*options),value),save_export);return
        try:
            j=named_snapshot(capture(state['memory'],exact.get(),day.get()),name.get())
            save_export(j)
        except Exception as e:messagebox.showerror('Capture unavailable',str(e))
    def save_export(j):
        try:
            path=filedialog.asksaveasfilename(title='Save route checkpoint',initialdir=str(output_dir()),initialfile=j['_capture']['checkpointName']+'.json',defaultextension='.json',filetypes=[('WWHD memfile','*.json')])
            if not path:return
            if Path(path).exists():messagebox.showinfo('Choose a new filename','Use a new filename to keep your existing checkpoint.');return
            save_json(path,j)
            try:
                append_capture(output_dir()/'capture-log.jsonl',j,path,state['memory'].pid)
                status.set('Saved '+str(path)+' • Full details appended to '+str(output_dir()/'capture-log.jsonl'))
            except Exception as e:
                messagebox.showwarning('Memfile saved; log unavailable','The JSON was saved at '+str(path)+'. Log write failed: '+str(e))
                status.set('Memfile saved; log failed. Share the JSON instead.')
            state['saved_message']=True
        except Exception as e:messagebox.showerror('Capture unavailable',str(e))
    capture_button=ttk.Button(frame,text='Capture memfile…',command=export,state='disabled');capture_button.pack(anchor='w',pady=5)
    ttk.Label(frame,textvariable=status,wraplength=690).pack(anchor='w',pady=(12,0))
    ttk.Label(frame,text='Read-only capture. Every export appends its full data to Captures/capture-log.jsonl.',foreground='#555555').pack(anchor='w',side='bottom')
    def show_snapshot(j):
            state['last']=j
            details.set(f"{j['stage']}    Room {j['roomId']}    Spawn {j['spawnId']}    Layer {j['layer']}    Animation set {j['_capture']['animationSet']}\n"+
                        'Position '+', '.join(f'{v:.2f}' for v in j['position'])+f"    Facing {j['angle']*360/65536:.2f}°")
            capture_button.configure(state='normal')
            label=('Wii U '+state['memory'].ip) if isinstance(state['memory'],WiiUMemory) else ('Cemu PID '+str(state['memory'].pid))
            if not state.pop('saved_message',False):status.set('Ready • '+label+' • current state read again at export')
    def poll(force=False):
        if not state['memory'] or state['busy']:return
        if isinstance(state['memory'],WiiUMemory):
            if not force and time.monotonic()<state['next_poll']:return
            state['next_poll']=time.monotonic()+5;memory=state['memory'];options=(exact.get(),day.get())
            submit(lambda:capture(memory,*options),show_snapshot);return
        try:
            show_snapshot(capture(state['memory'],exact.get(),day.get()))
        except Exception as e:capture_button.configure(state='disabled');details.set('No playable snapshot available');status.set(str(e))
    def tick():
        while not results.empty():
            generation,callback,value,error=results.get()
            if generation!=state['generation']:
                if isinstance(value,WiiUMemory):value.close()
                continue
            state['busy']=False
            if error:
                state['last']=None
                memory=state['memory']
                connected=memory is not None and (not isinstance(memory,WiiUMemory) or memory.sock is not None)
                if connected:
                    state['next_poll']=time.monotonic()+5
                    status.set(str(error)+' Still connected; checking again automatically. You can also press Capture or Refresh game state.')
                    details.set('Waiting for a stable current snapshot. No earlier snapshot will be exported.')
                    capture_button.configure(state='normal')
                else:
                    state['next_poll']=float('inf');status.set(str(error))
                    details.set('Connection unavailable. Connect again when the game is running.')
                    capture_button.configure(state='disabled')
            else:callback(value)
        poll();root.after(300,tick)
    def close():disconnect();root.destroy()
    root.protocol('WM_DELETE_WINDOW',close);refresh()
    if smoke_report:
        root.update_idletasks()
        Path(smoke_report).write_text(json.dumps({'tkStarted':True,'nameFieldPresent':True,'fullAppendOnlyCaptureLog':True,'processCount':len(state['processes']),
            'wiiUDeviceChoice':True,'runtimeProfileField':True,'ipField':ip.get(),'networkOperationsOffUiThread':True,
            'connected':state['memory'] is not None,'captureButtonEnabled':str(capture_button['state'])!='disabled',
            'status':status.get(),'requiredSize':[root.winfo_reqwidth(),root.winfo_reqheight()]},indent=2),encoding='utf-8')
        close();return
    if test_driver:
        root.after(0,lambda:test_driver(root,dict(state=state,source=source,ip=ip,profile=profile_address,name=name,connect=connect,export=export,poll=poll,status=status,details=details,capture_button=capture_button,close=close)))
    root.after(300,tick);root.mainloop()

def output_dir():
    folder=Path(sys.executable if getattr(sys,'frozen',False) else __file__).resolve().parent/'Captures'
    folder.mkdir(exist_ok=True);return folder

def main():
    p=argparse.ArgumentParser(description='Read-only WWHD EUR v0 memfile creator for Cemu and Wii U')
    p.add_argument('--list',action='store_true');p.add_argument('--pid',type=int);p.add_argument('--log',type=Path)
    p.add_argument('--probe',action='store_true',help='Read-only connection/profile check without capturing a memfile');p.add_argument('--capture',type=Path);p.add_argument('--check',action='store_true');p.add_argument('--spawn-only',action='store_true');p.add_argument('--daytime',action='store_true');p.add_argument('--report',type=Path)
    p.add_argument('--diagnose',action='store_true',help='Read GZ v75 capture guard and typed actor summary without exporting a memfile')
    p.add_argument('--gz-profile',help='Decimal or hexadecimal Creator profile number from the GZ v75 main menu')
    p.add_argument('--name',help='Checkpoint name stored in the JSON and capture log')
    p.add_argument('--wiiu-ip',help='Wii U IPv4 address; uses Aroma 7332 first, then legacy 7331')
    p.add_argument('--ui-smoke-report',type=Path,help=argparse.SUPPRESS)
    args=p.parse_args();report=None
    try:
        if args.ui_smoke_report:gui(args.ui_smoke_report);return 0
        if args.list:report={'cemuProcesses':[{'pid':pid,'path':path} for pid,path in processes()]}
        elif args.pid or args.wiiu_ip:
            if args.pid and args.wiiu_ip:p.error('Choose --pid or --wiiu-ip, not both')
            m=WiiUMemory(args.wiiu_ip,profile_address=args.gz_profile) if args.wiiu_ip else CemuMemory(args.pid,args.log)
            try:
                if args.diagnose:
                    from profile_diagnostics import diagnose
                    report=diagnose(m)
                    if args.report:args.report.write_text(json.dumps(report,indent=2),encoding='utf-8')
                    if sys.stdout:print(json.dumps(report,indent=2))
                    return 0
                if args.probe:
                    report={'connected':True,'source':getattr(m,'source',{'device':'Cemu'}),'stage':m.read(0x104741e4,8).split(b'\0')[0].decode(errors='replace'),'captured':None}
                    if args.report:args.report.write_text(json.dumps(report,indent=2),encoding='utf-8')
                    return 0
                j=capture(m,not args.spawn_only,args.daytime)
                if args.capture:
                    j=named_snapshot(j,args.name or args.capture.stem)
                    save_json(args.capture,j)
                    append_capture(output_dir()/'capture-log.jsonl',j,args.capture,args.pid)
                report={'connected':True,'pid':args.pid,'wiiUip':args.wiiu_ip,'log':m.log,'captured':str(args.capture) if args.capture else None,'snapshot':j}
            finally:m.close()
        elif args.check or args.capture or args.probe:p.error('--pid or --wiiu-ip is required for --check or --capture')
        else:gui();return 0
        if args.report:args.report.write_text(json.dumps(report,indent=2),encoding='utf-8')
        if sys.stdout:print(json.dumps(report,indent=2))
        return 0
    except Exception as e:
        report={'error':str(e)}
        if args.report:args.report.write_text(json.dumps(report,indent=2),encoding='utf-8')
        if sys.stderr:print(str(e),file=sys.stderr)
        return 1

if __name__=='__main__':sys.exit(main())
