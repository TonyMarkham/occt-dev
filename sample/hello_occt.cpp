// Headless OCCT validation: build a 10 x 20 x 30 mm box, write it to a STEP
// file, read the file back, and assert the volume survives the round trip.
//
// Expected output (exit code 0):
//   STEP written: hello_occt.step
//   STEP read back: 1 root(s)
//   Volume: 6000 mm^3

#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <GProp_GProps.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_StepModelType.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include <cmath>
#include <iostream>

int main()
{
  // OCCT 8.0.1 prints a "Statistics on Transfer (Write)" banner
  // unconditionally on the default messenger during STEP transfer.
  // Clear its printers so the output below stays deterministic.
  Message::DefaultMessenger()->ChangePrinters().Clear();

  // OCCT 8.0 Make* classes construct lazily: the parameterized constructor
  // only prepares the primitive; Build() does the work.
  BRepPrimAPI_MakeBox aBoxMaker (10.0, 20.0, 30.0);
  aBoxMaker.Build();
  if (!aBoxMaker.IsDone())
  {
    std::cerr << "hello_occt: FAILED: box construction not done" << std::endl;
    return 1;
  }
  const TopoDS_Shape aBox = aBoxMaker.Shape();

  STEPControl_Writer aWriter;
  if (aWriter.Transfer (aBox, STEPControl_AsIs) != IFSelect_RetDone)
  {
    std::cerr << "hello_occt: FAILED: STEP transfer" << std::endl;
    return 1;
  }
  if (aWriter.Write ("hello_occt.step") != IFSelect_RetDone)
  {
    std::cerr << "hello_occt: FAILED: STEP write" << std::endl;
    return 1;
  }
  std::cout << "STEP written: hello_occt.step" << std::endl;

  STEPControl_Reader aReader;
  if (aReader.ReadFile ("hello_occt.step") != IFSelect_RetDone)
  {
    std::cerr << "hello_occt: FAILED: STEP open" << std::endl;
    return 1;
  }
  const int aNbRoots = aReader.TransferRoots();
  if (aNbRoots != 1)
  {
    std::cerr << "hello_occt: FAILED: STEP read back produced " << aNbRoots
              << " root(s)" << std::endl;
    return 1;
  }
  const TopoDS_Shape aReadBack = aReader.OneShape();
  if (aReadBack.IsNull())
  {
    std::cerr << "hello_occt: FAILED: STEP read back shape is null" << std::endl;
    return 1;
  }

  GProp_GProps aProps;
  BRepGProp::VolumeProperties (aReadBack, aProps);
  const double aVolume = aProps.Mass();

  std::cout << "STEP read back: " << aNbRoots << " root(s)" << std::endl;
  std::cout << "Volume: " << aVolume << " mm^3" << std::endl;

  if (std::abs (aVolume - 6000.0) > 0.01)
  {
    std::cerr << "hello_occt: FAILED: expected volume 6000, got " << aVolume
              << std::endl;
    return 1;
  }
  return 0;
}
