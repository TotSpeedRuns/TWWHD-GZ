from pathlib import Path
import hashlib,sys,tempfile,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import sync_components as sync

class ComponentSyncTests(unittest.TestCase):
    def test_traversal_and_non_source_destinations_refused(self):
        for name in ('../keys.txt','experiments/maintained-components/../../runtime/x.c',
                     'experiments/maintained-components/x.rpx','C:/outside.c',
                     'experiments\\maintained-components\\x.c'):
            with self.subTest(name=name),self.assertRaises(ValueError):sync.destination(name)

    def test_changed_source_refused_before_any_destination_written(self):
        with tempfile.TemporaryDirectory()as d:
            root=Path(d);source=root/'source.c';source.write_text('int a;\n')
            target=sync.ROOT/'experiments/maintained-components/never-written-test.c'
            self.assertFalse(target.exists())
            with self.assertRaises(ValueError):
                sync.sync(dict(revision='R-test',files=[dict(source=str(source),destination=target.relative_to(sync.ROOT).as_posix(),sha256='0'*64)]))
            self.assertFalse(target.exists())

    def test_binary_source_refused_before_copy(self):
        with tempfile.TemporaryDirectory()as d:
            source=Path(d)/'bad.c';data=b'raw\0capture';source.write_bytes(data)
            with self.assertRaises(ValueError):
                sync.sync(dict(revision='R-test',files=[dict(source=str(source),destination='experiments/maintained-components/never-written-test.c',sha256=hashlib.sha256(data).hexdigest())]))
