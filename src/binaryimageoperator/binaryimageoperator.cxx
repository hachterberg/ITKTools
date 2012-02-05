/*=========================================================================
*
* Copyright Marius Staring, Stefan Klein, David Doria. 2011.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0.txt
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*=========================================================================*/
/** \file
 \brief Apply a binary operator to a pair of images.
 
 \verbinclude binaryimageoperator.help
 */

#include "itkCommandLineArgumentParser.h"
#include "ITKToolsHelpers.h"
#include "ITKToolsBase.h"
#include "ITKToolsImageProperties.h"

#include "BinaryImageOperatorMainHelper.h"
#include "BinaryImageOperatorHelper.h"


/**
 * ******************* GetHelpString *******************
 */

std::string GetHelpString( void )
{
  std::stringstream  ss;
  ss << "ITKTools v" << itktools::GetITKToolsVersion() << "\n"
    << "Performs binary operations on two images." << std::endl
    << "Usage:\npxbinaryimageoperator\n"
    << "  -in      inputFilenames\n"
    << "  [-out]   outputFilename, default in1 + ops + arg + in2 + .mhd\n"
    << "  -ops     binary operator of the following form:\n"
    << "           {+,-,*,/,^,%}\n"
    << "           notation:\n"
    << "             {ADDITION, WEIGHTEDADDITION, MINUS, TIMES,DIVIDE,POWER,\n"
    << "             MAXIMUM, MINIMUM, ABSOLUTEDIFFERENCE, SQUAREDDIFFERENCE,\n"
    << "             BINARYMAGNITUDE, MASK, MASKNEGATED, LOG}\n"
    << "           notation examples:\n"
    << "             MINUS = A - B\n"
    << "             ABSDIFF = |A - B|\n"
    << "             MIN = min( A, B )\n"
    << "             MAGNITUDE = sqrt( A * A + B * B )\n"
    << "  [-arg]   argument, necessary for some ops\n"
    << "             WEIGHTEDADDITION: 0.0 < weight alpha < 1.0\n"
    << "             MASK[NEG]: background value, e.g. 0.\n"
    << "  [-z]     compression flag; if provided, the output image is compressed\n"
    << "  [-opct]  output component type, by default the largest of the two input images\n"
    << "           choose one of: {[unsigned_]{char,short,int,long},float,double}\n"
    << "Supported: 2D, 3D, (unsigned) char, (unsigned) short, (unsigned) int, (unsigned) long, float, double.";
  return ss.str();

} // end GetHelpString()


//-------------------------------------------------------------------------------------

int main( int argc, char **argv )
{
  /** Create a command line argument parser. */
  itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
  parser->SetCommandLineArguments( argc, argv );
  parser->SetProgramHelpText( GetHelpString() );
  parser->MarkArgumentAsRequired( "-in", "The input filename." );
  parser->MarkArgumentAsRequired( "-ops", "The operation to perform." );

  itk::CommandLineArgumentParser::ReturnValue validateArguments = parser->CheckForRequiredArguments();

  if ( validateArguments == itk::CommandLineArgumentParser::FAILED )
  {
    return EXIT_FAILURE;
  }
  else if ( validateArguments == itk::CommandLineArgumentParser::HELPREQUESTED )
  {
    return EXIT_SUCCESS;
  }
  
  /** Get arguments. */
  std::vector<std::string> inputFileNames;
  parser->GetCommandLineArgument( "-in", inputFileNames );

  // You must specify two input files.
  if ( inputFileNames.size() != 2 )
  {
    std::cerr << "ERROR: You should specify two input file names." << std::endl;
    return 1;
  }
  
  std::string outputFileName = "";
  parser->GetCommandLineArgument( "-out", outputFileName );

  std::string ops = "";
  parser->GetCommandLineArgument( "-ops", ops );

  std::string argument = "0";
  bool retarg = parser->GetCommandLineArgument( "-arg", argument );

  std::string outputComponentTypeString = "";
  bool retopct = parser->GetCommandLineArgument( "-opct", outputComponentTypeString );
  
  const bool useCompression = parser->ArgumentExists( "-z" );

  /** Create outputFileName. */
  if ( outputFileName == "" )
  {
    CreateOutputFileName( inputFileNames, outputFileName, ops, argument );
  }

  /** Determine image properties. */
  itktools::ComponentType componentType1;
  itktools::ComponentType componentType2;
  itktools::ComponentType componentTypeOut;
  DetermineComponentTypes( inputFileNames, componentType1, componentType2, componentTypeOut );

  /** Let the user override the output component type. */
  if ( retopct )
  {
    componentTypeOut = itk::ImageIOBase::GetComponentTypeFromString( outputComponentTypeString );
    if ( !itktools::ComponentTypeIsValid( componentTypeOut ) )
    {
      std::cerr << "ERROR: the you specified a wrong opct." << std::endl;
      return 1;
    }
    if ( !itktools::ComponentTypeIsInteger( componentTypeOut ) )
    {
      componentType1 = componentType2 = itk::ImageIOBase::DOUBLE;
    }
  }

  /** Check if a valid operator is given. */
  std::string opsCopy = ops;
  int retCO  = CheckOperator( ops );
  if ( retCO ) return retCO;

  /** For certain ops an argument is mandatory. */
  bool retCOA = CheckOperatorAndArgument( ops, argument, retarg );
  if ( !retCOA ) return 1;

  /** Class that does the work */
  ITKToolsBinaryImageOperatorBase * binaryImageOperator = NULL;

  unsigned int dim = 0;
  itktools::GetImageDimension( inputFileNames[1], dim );

  try
  {
    // now call all possible template combinations.
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, char, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned char, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, short, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned short, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, int, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned int, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, long, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned long, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< double, double, float, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< double, double, double, 2 >::New( componentType1, componentType2, componentTypeOut, dim );
#ifdef ITKTOOLS_3D_SUPPORT
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, char, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned char, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, short, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned short, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, int, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned int, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, long, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< long, long, unsigned long, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< double, double, float, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
    if (!binaryImageOperator) binaryImageOperator = ITKToolsBinaryImageOperator< double, double, double, 3 >::New( componentType1, componentType2, componentTypeOut, dim );
#endif
    if (!binaryImageOperator)
    {
      typedef itk::ImageIOBase                        ImageIOBaseType;
      std::cerr << "ERROR: this combination of pixeltype and dimension is not supported!" << std::endl;
      std::cerr
        << "  input1 pixel (component) type = " << itk::ImageIOBase::GetComponentTypeAsString( componentType1 )
        << "\n  input2 pixel (component) type = " << itk::ImageIOBase::GetComponentTypeAsString( componentType2 )
        << "\n  output pixel (component) type = " << itk::ImageIOBase::GetComponentTypeAsString( componentTypeOut )
        << "\n  dimension = " << dim << std::endl;
      return 1;
    }

    binaryImageOperator->m_InputFileName1 = inputFileNames[0];
    binaryImageOperator->m_InputFileName2 = inputFileNames[1];
    binaryImageOperator->m_OutputFileName = outputFileName;
    binaryImageOperator->m_Ops = ops;
    binaryImageOperator->m_UseCompression = useCompression;
    binaryImageOperator->m_Arg = argument;
  
    binaryImageOperator->Run();

    delete binaryImageOperator;
  }
  catch( itk::ExceptionObject &e )
  {
    std::cerr << "Caught ITK exception: " << e << std::endl;
    delete binaryImageOperator;
    return 1;
  }

  /** End program. */
  return 0;

} // end main
