use std::fs;
//use std::env;
use std::fs::File;
use std::io::Read;


use image::io::Reader;
use std::io::Cursor;



use image::{GenericImage, GenericImageView, ImageBuffer, RgbImage};


static FILE_NAME: &str = "/home/aragusa/CLAM-1485/pdf01";

fn easy(){

//    println!("Hello, world!");

    //let img  = image::open("/home/aragusa/CLAM-1485/image-2021-05-27-10-17-35-543.png").unwrap();
    let img  = image::open("/home/aragusa/CLAM-1485/pdf01").unwrap();

    // The dimensions method returns the images width and height.
    println!("dimensions {:?}", img.dimensions());

    // The color method returns the image's `ColorType`.
    println!("{:?}", img.color());

    // Write the contents of this image to the Writer in PNG format.
    img.save("ftt.jpg").unwrap();
}

fn dump(buffer: &Vec<u8>){

    let mut cnt = 0;

    for i in buffer {
        print!("{:02x} ", i);
        if 0 == (cnt % 16) {
            if cnt > 0{
                print!("\n");
            }
        }
        //cnt = cnt + 1;
        cnt += 1;
    }

    if 0 != (cnt % 16) {
                print!("\n");
    }


}

fn reader(file_name: String){

    println!("file_name = {}", file_name);

    //let contents = fs::read_to_string(file_name).unwrap();

    let mut f = File::open(&file_name).expect("no file found");
    let metadata = fs::metadata(&file_name).expect("unable to read metadata");
    let mut buffer = vec![0; metadata.len() as usize];
    f.read(&mut buffer).expect("buffer overflow");

//    dump(&buffer);


    let data = Cursor::new(buffer);
    let reader =Reader::new(data).with_guessed_format().expect("This will never fail using Cursor");

    let img = reader.decode().expect("Failed to read image");



}

fn main() {
//    easy();

    reader(FILE_NAME.to_string());
}




