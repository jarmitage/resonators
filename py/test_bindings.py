import argparse
import resonators
import traceback

def main(args):
    print('Testing basic bindings capabilities....')
    try:
        model = resonators.ModelLoader()
        model.load(args.model_path)
        rb_ops = resonators.ResonatorBankOptions()
        rb_ops.total = model.getSize()
        rb = resonators.ResonatorBank()
        rb.setup(rb_ops, 44100.0, 128)
        rb.setBank(model.getShiftedToNote("c4"))
        rb.update()
        out = rb.render(1.0)
    except Exception:
        print('Bindings not built correctly:')
        traceback.print_exc()
    else:
        print('Successful bindings build!')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('model_path', help='Path of model json')
    args = parser.parse_args()
    main(args)
